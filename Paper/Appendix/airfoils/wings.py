from math import factorial, cos, pi, sqrt

def a(A, t):
	if t < 0.5: s = 0; x = 1-2*t;
	else:       s = 1; x = 2*t-1;
	return (x, C(x) * S(A, x, s))
def C(x):
	return sqrt(x)*(1-x)
def S(A, x, s):
	return sum([A[s][i] * B(x, i, 7) for i in range(8)])
def B(x, i, n):
	return K(i, n) * x**i * (1-x)**(n-i)
def K(i, n):
	return factorial(n)/(factorial(i) * factorial(n-i))
def eucl(a, b):
	return sqrt((a[0]-b[0])**2 + (a[1]-b[1])**2)

wings = {
"BOEING737" : [[0.106824859496518, 0.302484647720715, -0.130657626895277, 0.563457719066398, -0.157407695490687, 0.290776204646226, 0.123026001991523, 0.152920483616188],[-0.106824859496518, 0.082553209643307, -0.417255662092407, 0.402251655603979, -0.756858659150221, 0.551211922056091, -0.529976983555885, 0.068653915036146]],
"CLARKY" : [[0.160688579000288, 0.339648552497354, 0.085059110226180, 0.414522127768049, 0.091415361514562, 0.359086660057675, 0.176538813864694, 0.280081052805736],[-0.160688579000288, -0.020114542505414, -0.164102609223028, 0.069886896183722, -0.151246466852484, 0.007918199293032, -0.050917044463767, -0.053538112278302]],
"EPPLER376" : [[0.104203021468383, 0.497108567946597, -0.293929451732845, 1.056692281786984, -0.605726564734741, 0.742445956045416, -0.078915859301677, 0.198538614910401],[-0.104203021468383, 0.619181344761484, -0.400674320051033, 1.110646875345319, -0.661856257021618, 0.756070771701362, -0.071977627685693, 0.148659213764075]],
"EPPLER545" : [[0.139402085433441, 0.463164825369011, -0.167157133589986, 0.918920431687647, -0.340836427301355, 0.719505717191583, 0.000539918308088, 0.355582518795301],[-0.139402085433441, -0.092260721012993, -0.345972635428599, 0.159118781594433, -0.717144001748148, 0.259784659159363, -0.272705516410125, 0.117559089208028]],
"GOTTINGEN459" : [[0.164700740905595, 0.192711229883103, 0.111227624227007, 0.245145331812178, 0.079047668713161, 0.235694783648492, 0.096876054834576, 0.212842358507005],[-0.164700740905595, -0.192711147281077, -0.111229550169061, -0.245140044687701, -0.079054973575148, -0.235689385117733, -0.096878258353289, -0.212841609584691]],
"GOTTINGEN702" : [[0.256210867424672, 0.519755412562355, 0.077240013675806, 0.741214060681712, -0.140414558654239, 0.652610911581345, -0.025174278019394, 0.219773653920648],[-0.256210867424672, 0.065558314552331, -0.498870172507034, 0.590602865865238, -0.686363817602765, 0.462379942996812, -0.323028692683264, 0.034639251003864]],
"NACA2412" : [[0.173428760696377, 0.265127513457339, 0.060719790791620, 0.459979801100276, -0.098912413695593, 0.455635379476169, 0.055012634763176, 0.294835954353467],[-0.173428760696377, -0.043518581932241, -0.274003847303634, 0.167742383821109, -0.372031628485471, 0.121201156553026, -0.168937260455300, -0.073168175634189]],
"NACA63206" : [[0.070211481920671, 0.127410766196012, 0.030716360114657, 0.222983801517003, -0.003917651524232, 0.195430081209479, 0.051748550866369, 0.105764056209768],[-0.070211481920671, 0.001673354859679, -0.160018000251468, 0.102133178366260, -0.219011855271342, 0.106770922919914, -0.099955483291463, 0.087993147946905]],
"RAE2822" : [[0.127479118968876, 0.130104791831467, 0.174592444953781, 0.125080114025995, 0.235828801050800, 0.167114493618810, 0.199301587596396, 0.205980460203035],[-0.127479118968876, -0.149863484551086, -0.105533890484132, -0.260572797774016, -0.091991172110012, -0.107187543147306, -0.057129716896824, 0.062478401673033]],
"NACA0012" : [[.17, .16, .152, .155, .145, .135, .14, .14], [-.17, -.16, -.152, -.155, -.145, -.135, -.14, -.14]]
}

for k in wings.keys():
	with open(k.lower()+".dat", "w") as f:
		n = 1000
		ts = [(cos(i/n*2*pi)-1)/-4 if i/n < 0.5 else (cos(i/n*2*pi)+3)/4 for i in range(n)]
		w = [a(wings[k], t) for t in ts]
		f.write("1 0\n")
		[f.write(str(w[i][0])+" "+str((w[i][1]+w[-i][1])/2)+"\n") for i in range(1, n//2)]
		f.write("0 0\n\n\n")
		[f.write(str(w[i][0])+" "+str(w[i][1])+"\n") for i in range(n)]
		f.write("1 0\n")