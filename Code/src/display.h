class display
{
	private:
	typedef std::list<vertex>::iterator _Vertex;
	typedef std::list<vertex>::reverse_iterator _RVertex;
	typedef std::list<triangle>::iterator _Triangle;
	typedef std::list<triangle>::reverse_iterator _RTriangle;


	/* === Variables === */
	private:

	/* Window */
	SDL_Window *_window;
	SDL_GLContext _context[2];
	int _window_w, _window_h, _screen_w, _screen_h, _program;

	/* Render thread */
	bool _running = 1;
	std::thread _renderer;
	std::mutex _mutex_cond, _mutex_data;
	std::condition_variable_any _cond_main, _cond_renderer;

	/* Camera */
	float _cam_x = 0.5, _cam_y = 0, _cam_ratio, _cam_zoom = 1.1;

	/* Transformations / Shader data */
	int _projection_location;

	/* Mouse */
	bool _left_button = 0, _right_button = 0;
	int _mouse_x, _mouse_y;

	/* Flags and objects data */
	mesh &_mesh;
	unsigned int _vao, _vbo_vertices, _vbo_indices, _display_size = 0;
	unsigned int _allocated_vertices = 0, _allocated_indices = 0, _size_vertices = 0, _size_indices = 0;
	float *_vertices = nullptr; unsigned int *_indices = nullptr;
	unsigned long int _last_vertex_id = 0;
	float _allocation_factor = 1.5;
	/* === Variables === */


	/* === Constructor / Destructor === */
	public:
	display (mesh &mesh_) : _mesh(mesh_)
	{
		signal(SIGINT, exit);

		/* Window */
		Display *display = XOpenDisplay(NULL);
		Screen *screen = DefaultScreenOfDisplay(display);
		_window_w = _screen_w = screen->width;
		_window_h = _screen_h = screen->height;
		XCloseDisplay(display);

		/* SDL */
		SDL_Init(SDL_INIT_VIDEO);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
		_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _window_w, _window_h,
		                           SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
		_context[0] = SDL_GL_CreateContext(_window);
		_context[1] = SDL_GL_CreateContext(_window);

		/* GLEW */
		glewExperimental = GL_TRUE;
		glewInit();

		/* Open GL */
		glEnable(GL_MULTISAMPLE);
		glViewport(0, 0, _window_w, _window_h);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		/* Vertex Shader */
		const char *vertex_src = "\
			#version 330\n\
			layout (location = 0) in vec2 position;\
			uniform mat4 projection;\
			void main() {\
				gl_Position = projection * vec4(position, 0.0, 1.0);\
			}";
		int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex_shader, 1, (const GLchar**)&vertex_src, NULL);
		glCompileShader(vertex_shader);

		/* Fragment Shader */
		const char *fragment_src = "\
			#version 330\n\
			uniform vec4 object_colour;\
			out vec4 colour;\
			void main(){\
        		colour = object_colour;\
			}";
		int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment_shader, 1, (const GLchar**)&fragment_src, NULL);
		glCompileShader(fragment_shader);

		/* Program */
		_program = glCreateProgram();
		glAttachShader(_program, vertex_shader);
		glAttachShader(_program, fragment_shader);
		glLinkProgram(_program); glUseProgram(_program);
		glUniform4f(glGetUniformLocation(_program, "object_colour"), 1, 1, 1, 1);
		_projection_location = glGetUniformLocation(_program, "projection");
		_cam_ratio = 1.35 / _window_w;
		_update_projection();

		/* Free */
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);

		/* Generate Buffers (VAO and VBOs) */
		glGenVertexArrays(1, &_vao);     glBindVertexArray(_vao);
		glGenBuffers(1, &_vbo_vertices); glBindBuffer(GL_ARRAY_BUFFER,         _vbo_vertices);
		glGenBuffers(1, &_vbo_indices);  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_indices);
		/* Define VAO */
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		/* Create render thread */
		_mutex_cond.lock();
		_renderer = std::thread(&display::_render, this);
		_cond_main.wait(_mutex_cond);
		_mutex_cond.unlock();

		/* Bind VBOs on the main thread */
		SDL_GL_MakeCurrent(_window, _context[0]);
		glBindBuffer(GL_ARRAY_BUFFER,         _vbo_vertices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_indices);
	}

	~display ()
	{
		if(_allocated_vertices > 0) free(_vertices);
		if(_allocated_indices > 0) free(_indices);
		_running = 0;
		_mutex_cond.lock();
		_cond_renderer.notify_one();
		_mutex_cond.unlock();
		_renderer.join();
		glDeleteVertexArrays(1, &_vao);
		glDeleteBuffers(1, &_vbo_vertices);
		glDeleteBuffers(1, &_vbo_indices);
		glDeleteProgram(_program);
		SDL_GL_DeleteContext(_context);
		SDL_DestroyWindow(_window);
		SDL_Quit();
	}
	/* === Constructor / Destructor === */


	/* #################################################################### */
	/* ###################### Render thread notifier ###################### */
	/* === Notify the render thread that new data is to be displayed === */
	private:
	void
	_notify_renderer (bool mode)
	{
		_mutex_cond.lock();
		_cond_renderer.notify_one();
		if(mode == 0) _cond_main.wait(_mutex_cond);
		_mutex_cond.unlock();
	}
	/* === Notify the render thread that new data is to be displayed === */
	/* ###################### Render thread notifier ###################### */
	/* #################################################################### */


	/* ##################################################################### */
	/* ######################### Process mesh data ######################### */
	/* === Pre processing - manage memory allocation === */
	private:
	bool
	_pre_process (bool &alloc_vertices, bool &alloc_indices)
	{
		if(_allocated_indices < _mesh._triangles.size() * 3) {
			_allocated_indices = _mesh._triangles.size() * 3 *_allocation_factor;
			_indices = (unsigned int *) realloc(_indices, _allocated_indices * sizeof(unsigned int));
			alloc_indices = true;
		} else alloc_indices = false;

		// Maybe reallocate and not redo, instead of not reallocate and redo (if usage is above 50% perhaps)
		unsigned int new_vertices = _mesh._vertices.rbegin()->_id - _last_vertex_id + 1;
		if(_allocated_vertices < _size_vertices + (new_vertices << 1)) {
			if(_allocated_vertices < _mesh._vertices.size() << 1) {
				_allocated_vertices = (_mesh._vertices.size() << 1) * _allocation_factor;
				_vertices = (float *) realloc(_vertices, _allocated_vertices * sizeof(float));
				alloc_vertices = true; return true;
			} else { alloc_vertices = false; return true; }
		} else { alloc_vertices = false; return false; }
	}
	/* === Pre processing - manage memory allocation === */


	/* === Process vertices - redo === */
	private:
	template <typename It>
	void
	_process_vertices_redo (It it, unsigned int begin, unsigned int end)
	{
		unsigned int display_id = begin >> 1;
		for(unsigned int i = begin; i < end; ++display_id, ++it) {
			it->_display_id = display_id;
			_vertices[i] = it->_x[0]; ++i;
			_vertices[i] = it->_x[1]; ++i;
		}
	}
	/* === Process vertices - redo === */

	/* === Process vertices - append new ones === */
	private:
	void
	_process_vertices_append (_RVertex it)
	{
		for(unsigned int i = _size_vertices >> 1; it->_id >= _last_vertex_id; ++i, ++it) {
			it->_display_id = i;
			_vertices[_size_vertices] = it->_x[0]; ++_size_vertices;
			_vertices[_size_vertices] = it->_x[1]; ++_size_vertices;
		}
	}
	/* === Process vertices - append new ones === */

	/* === Process triangles - indices === */
	private:
	template <typename It>
	void
	_process_indices (It it, unsigned int begin, unsigned int end)
	{
		for(unsigned int i = begin; i < end; ++it) {
			_indices[i] = it->_vertices[0]->_display_id; ++i;
			_indices[i] = it->_vertices[1]->_display_id; ++i;
			_indices[i] = it->_vertices[2]->_display_id; ++i;
		}
	}
	/* === Process triangles - indices === */


	/* === Define mesh display data === */
	public:
	void
	display_mesh (bool mode)
	{
		/* In case the current thread is not the one that performed the initialisation */
		// SDL_GL_MakeCurrent(_window, _context[0]);

		/* Set title */
		char title[50];
		sprintf(title, "Vertices: %lu  -  Triangles: %lu", _mesh._vertices.size(), _mesh._triangles.size()); 
		SDL_SetWindowTitle(_window, title);

		/* If there are no triangles to display - notify the renderer and exit */
		if(_mesh._triangles.size() == 0) {
			_mutex_data.lock(); _display_size = 0; _mutex_data.unlock();
			_notify_renderer(mode); return;
		}

		bool alloc_vertices, alloc_indices;
		unsigned int half;
		std::thread worker;

		/* USE ARRAYS INSTEAD OF VECTORS */

		/* Process vertices - redo */
		if(_pre_process(alloc_vertices, alloc_indices)) {
			_size_vertices = _mesh._vertices.size() << 1;
			half = _mesh._vertices.size() >> 1 << 1;
			worker = std::thread(&display::_process_vertices_redo<_Vertex>, this, _mesh._vertices.begin(), 0, half);
			_process_vertices_redo(_mesh._vertices.rbegin(), half, _mesh._vertices.size() << 1);
			worker.join();
		}
		/* Process vertices - append */
		else _process_vertices_append(_mesh._vertices.rbegin());
		_last_vertex_id = _mesh._vertices.rbegin()->_id + 1;

		/* Process indices */
		_size_indices = _mesh._triangles.size() * 3;
		half = (_mesh._triangles.size() >> 1) * 3;
		worker = std::thread(&display::_process_indices<_Triangle>, this, _mesh._triangles.begin(), 0, half);
		_process_indices(_mesh._triangles.rbegin(), half, _mesh._triangles.size() * 3);
		worker.join();

		/* - Update buffers - */
		_mutex_data.lock();

		_display_size = _size_indices;
		/* Reallocate space for buffers */
		if(alloc_vertices)
			glBufferData(GL_ARRAY_BUFFER, _allocated_vertices * sizeof(float), NULL, GL_DYNAMIC_DRAW);
		if(alloc_indices)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, _allocated_indices * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
		/* Update buffer data */
		glBufferSubData(GL_ARRAY_BUFFER, 0, _size_vertices * sizeof(float), &_vertices[0]);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _size_indices * sizeof(unsigned int), &_indices[0]);
		glFlush();

		_mutex_data.unlock();
		/* - Update buffers - */

		/* Notify renderer */
		_notify_renderer(mode);
	}
	/* === Define mesh display data === */
	/* ######################### Process mesh data ######################### */
	/* ##################################################################### */


	/* ##################################################################### */
	/* ###################### Transformation Matrices ###################### */
	/* === Update projection transformation matrix === */
	private:
	void
	_update_projection ()
	{
		float _1 = _cam_ratio/2; float _2 = _window_w * _1, _3 = _window_h * _1;
		glUniformMatrix4fv(_projection_location, 1, GL_FALSE,
			               &glm::ortho(_cam_x-_2, _cam_x+_2, _cam_y-_3, _cam_y+_3, (float)-1, (float)1)[0][0]);
	}
	/* === Update projection transformation matrix === */
	/* ###################### Transformation Matrices ###################### */
	/* ##################################################################### */


	/* ##################################################################### */
	/* ############################# Rendering ############################# */
	/* === Render thread function === */
	private:
	void
	_render ()
	{
		bool rendering;
		SDL_Event event;

		SDL_GL_MakeCurrent(_window, _context[1]);

		_mutex_cond.lock();
		_cond_main.notify_one();
		while(_running) {
			/* Wait for main thread to notify */
			_cond_renderer.wait(_mutex_cond);
			SDL_ShowWindow(_window);
			_mutex_cond.unlock();
			if(!_running) break;

			rendering = 1;
			while(_running && rendering) {
				/* Manage events */
				while(SDL_PollEvent(&event))
					switch(event.type) {
						case SDL_MOUSEMOTION: case SDL_MOUSEBUTTONUP:
						case SDL_MOUSEWHEEL:  case SDL_MOUSEBUTTONDOWN: _mouse(event); break;
						case SDL_KEYUP: case SDL_KEYDOWN: _keyboard(event); break;
						case SDL_WINDOWEVENT:
							if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
								_window_w = event.window.data1; _window_h = event.window.data2;
								_mutex_data.lock();
								glViewport(0, 0, _window_w, _window_h); _update_projection();
								_mutex_data.unlock(); }
							break;
						case SDL_QUIT: rendering = 0; SDL_HideWindow(_window); break;
					}
				/* Draw scene */
				_draw();
			}
			_mutex_cond.lock();
			_cond_main.notify_one();
			if(!_running) _mutex_cond.unlock();
		}
	}
	/* === Renderer thread function === */


	/* === Drawing function === */
	private:
	void
	_draw ()
	{
		/* Reset display */
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Draw Triangles */
		_mutex_data.lock();
		glDrawElements(GL_TRIANGLES, _display_size, GL_UNSIGNED_INT, 0);
		_mutex_data.unlock();

		SDL_GL_SwapWindow(_window);
	}
	/* === Drawing function === */
	/* ############################# Rendering ############################# */
	/* ##################################################################### */


	/* ##################################################################### */
	/* ###################### Keyboard/Mouse handling ###################### */
	/* === Keyboard handling === */
	private:
	void
	_keyboard (SDL_Event &event)
	{
		if(event.key.state == SDL_PRESSED)
			switch(event.key.keysym.sym) {
				/* R key - reset scene */
				case SDLK_r: _cam_x = 0.5; _cam_y = 0; _cam_ratio = 1.35/_window_w;
				             _mutex_data.lock(); _update_projection(); _mutex_data.unlock(); break;
				/* Space key - notify main thread */
				case SDLK_SPACE: _mutex_cond.lock(); _cond_main.notify_one(); _mutex_cond.unlock(); break;
			}
	}
	/* === Keyboard handling === */


	/* === Mouse handling === */
	private:
	void
	_mouse (SDL_Event &event)
	{
		SDL_MouseMotionEvent *motion = &event.motion;
		SDL_MouseButtonEvent *button = &event.button;

		switch(event.type) {
			case SDL_MOUSEMOTION:
				_mouse_x = motion->x; _mouse_y = motion->y;
				/* Mouse left-click - move camera */
				if(_left_button == 1) {
					_cam_x -= motion->xrel * _cam_ratio;
					_cam_y += motion->yrel * _cam_ratio;
				}
				_mutex_data.lock(); _update_projection(); _mutex_data.unlock();
				break;

			case SDL_MOUSEBUTTONUP:
				     if(button->button == SDL_BUTTON_LEFT)   _left_button = 0;
				else if(button->button == SDL_BUTTON_RIGHT) _right_button = 0;
				break;

			case SDL_MOUSEBUTTONDOWN:
				     if(button->button == SDL_BUTTON_LEFT)   _left_button = 1;
				else if(button->button == SDL_BUTTON_RIGHT) _right_button = 1;
				_mouse_x = button->x; _mouse_y = button->y;
				break;

			/* Zoom - adjust distance from camera to object */
			case SDL_MOUSEWHEEL:
				if(button->x == 1) {
					_cam_x += (_mouse_x - (_window_w >> 1)) * _cam_ratio * (1-(1/_cam_zoom));
					_cam_y -= (_mouse_y - (_window_h >> 1)) * _cam_ratio * (1-(1/_cam_zoom));
			    	_cam_ratio /= _cam_zoom;
			    } else if(button->x == -1) {
					_cam_x -= (_mouse_x - (_window_w >> 1)) * _cam_ratio * (_cam_zoom-1);
					_cam_y += (_mouse_y - (_window_h >> 1)) * _cam_ratio * (_cam_zoom-1);
			    	_cam_ratio *= _cam_zoom;
			    }
				_mutex_data.lock(); _update_projection(); _mutex_data.unlock();
				break;
		}
	}
	/* === Mouse handling === */
	/* ###################### Keyboard/Mouse handling ###################### */
	/* ##################################################################### */
};
