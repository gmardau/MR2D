from numpy import array, mean
from copy import deepcopy
from itertools import product
from collections import OrderedDict

_w = OrderedDict((("boeing737",[0, "Boeing 737"]), ("clarky",[1, "Clark-Y"])\
	, ("eppler376",[2, "Eppler 376"]), ("eppler545",[3, "Eppler 545"])\
	, ("gottingen459",[4, "Gottingen 459"]), ("gottingen702",[5, "Gottingen 702"])\
	, ("naca2412",[6, "NACA 2412"]), ("naca63206",[7, "NACA 63206"]), ("rae2822",[8, "RAE 2822"])))
_s = OrderedDict((("0.01",[0, ".01"]), ("0.05",[1, ".05"]), ("0.25",[2, ".25"])))
_r = OrderedDict((("50-1",[0, ["50","1"]]), ("100-2",[1, ["100","2"]]), ("200-4",[2, ["200","4"]])\
    , ("300-6",[3, ["300","6"]]), ("400-8",[4, ["400","8"]]), ("700-14",[5, ["700","14"]])))
_d = OrderedDict((("-",[0, "-"]), ("0.5",[1, ".5"]), ("1",[2, "1"]), ("2",[3, "2"]), ("4",[4, "4"])))
_m = OrderedDict((("0",[0, "0"]), ("1",[1, "1"]), ("2",[2, "2"])))

_tr = [[[[[[] for i in range(len(_m))] for j in range(len(_d))] for k in range(len(_r))]\
      for l in range(len(_s))] for m in range(len(_w))]
_tm = deepcopy(_tr); _pr = deepcopy(_tr); _es = deepcopy(_tr); _su = deepcopy(_tr);

with open("../../../Code/output/all/results", "r") as f:
	lines = [l.split() for l in f.readlines()]
for l in lines:
	_tr[_w[l[0]][0]][_s[l[1]][0]][_r[l[2]][0]][_d[l[3]][0]][_m[l[4]][0]].append(float(l[7]))
	_tm[_w[l[0]][0]][_s[l[1]][0]][_r[l[2]][0]][_d[l[3]][0]][_m[l[4]][0]].append(float(l[8]))
	_pr[_w[l[0]][0]][_s[l[1]][0]][_r[l[2]][0]][_d[l[3]][0]][_m[l[4]][0]].append(float(l[9]))

_all = [range(len(_w)),range(len(_s)),range(len(_r)),range(len(_d)),range(len(_m))]
for i,j,k,l,m in product(*_all):
	if len(_tr[i][j][k][l][m]) > 0:
		_tr[i][j][k][l][m] = mean(array(_tr[i][j][k][l][m]))
		_tm[i][j][k][l][m] = mean(array(_tm[i][j][k][l][m]))
		_pr[i][j][k][l][m] = mean(array(_pr[i][j][k][l][m]))
		if m != 0:
			_es[i][j][k][l][m] = _tr[i][j][k][l][m]*100/_tr[i][j][k][0][0]-100
			_su[i][j][k][l][m] = _tm[i][j][k][0][0]/_tm[i][j][k][l][m]

_c4 = ["red!70","orange!70","green!60!black!70","blue!60!"]
#_c4 = ["black!25","black!50","black!75","black"]; #_c4.reverse()
_ar = [[_pr,"_pr","Preservation (\\%)"], [_es,"_es","Element surplus (\\%)"], [_su,"_su","Speed-up"]]

for i,w in list(enumerate(_w.keys())):
	for j,s in enumerate(_s.keys()):
		with open("../tables/"+w+"_"+s+".tex", "w") as f:
			f.write("\\begin{table}[!hp]\n\\begin{center}\n")
			f.write("\\begin{tabular}{c|cc?c|c?c?c|c|c||c|c}\n")
			f.write("\\multirow{2}{*}{\\textbf{\\large $\\boldsymbol{\\sigma}$}} & ")
			f.write("\\multirow{2}{*}{\\textbf{\\large $\\boldsymbol{I}$}} & ")
			f.write("\\multirow{2}{*}{\\textbf{\\large $\\boldsymbol{G}$}} & ")
			f.write("\\multicolumn{2}{c?}{\\textbf{\\large Generation}} & ")
			f.write("\\multirow{2}{*}{\\textbf{\\large $\\boldsymbol{D}$}} & ")
			f.write("\\multicolumn{5}{c}{\\textbf{{\\large Remodelling} }} ")
			f.write("\\\\\\cline{4-5}\\cline{7-11}\n")
			f.write(" & & & \\textbf{\\# Tri.} & \\textbf{Time} & &")
			f.write("\\textbf{\\# Tri.} & \\textbf{Time} & ")
			f.write("\\textbf{Preserv.} & \\textbf{+ Tri.} & \\textbf{Sp.-up} \\\\\\toprule\n")
			f.write("\\multirow{24}[11]{*}{"+_s[s][1]+"}")
			for k,r in enumerate(_r.keys()):
				f.write(" & \\multirow{4}{*}{"+_r[r][1][0]+"} & \\multirow{4}{*}{"+_r[r][1][1]+"}")
				f.write(" & \\multirow{4}{*}{"+str(int(round(_tr[i][j][k][0][0],0)))+"}")
				f.write(" & \\multirow{4}{*}{"+str('{0:.2f}'.format(round(_tm[i][j][k][0][0],2)))+" ms}")
				for l,d in list(enumerate(_d.keys()))[1:]:
					f.write(" & "+_d[d][1]+" & ")
					for m in range(2,3):
						f.write(str(int(round(_tr[i][j][k][l][m],0)))+" & ")
						f.write(str('{0:.2f}'.format(round(_tm[i][j][k][l][m],2)))+" ms & ")
						f.write(str('{0:.2f}'.format(round(_pr[i][j][k][l][m],2)))+" \\% & ")
						f.write(str('{0:.2f}'.format(round(_es[i][j][k][l][m],2)))+" \\% & ")
						f.write(str('{0:.2f}'.format(round(_su[i][j][k][l][m],2))))
						if(m == 1): f.write(" & ")
					if l != len(_d)-1: f.write(" \\\\\\cline{6-11}\n & & & & ")
					elif k != len(_r)-1: f.write(" \\\\\\cmidrule[1.5pt]{2-11}\n")
					else: f.write("\\\\\\bottomrule\n")
			f.write("\\end{tabular}\\end{center}\n")
			f.write("\\caption{Full results of mesh remodelling for $\sigma=" + s + "$ - "+ _w[w][1] + " airfoil}")
			f.write("\\centering\\sffamily\\footnotesize\nAverage of thirty optimisation scenarios")
			f.write("\\end{table}\n")


	for array,name,label in _ar:
		with open("../figures/plots/"+w+name+".tex", "w") as f:
			ymin = min([min([min([min([array[i][j][k][l][m] for k in range(len(_r))])\
				    for m in range(2,3)]) for l in range(1,len(_d))]) for j in range(len(_s))])
			ymax = max([max([max([max([array[i][j][k][l][m] for k in range(len(_r))])\
			        for m in range(2,3)]) for l in range(1,len(_d))]) for j in range(len(_s))])
			ymin = ymin-(ymax-ymin)/20
			ymax = ymax+(ymax-ymin)/20
			f.write("\\begin{figure}[!h]\n\\begin{center}\n\\begin{tikzpicture}\n")
			for j,s in enumerate(_s.keys()):
				f.write("\\begin{scope}[shift={("+str(0.28*j)+"\linewidth,0)}]\n")
				f.write("\\begin{axis}[\n width=0.34\linewidth,height=0.23\linewidth,\n")
				f.write(" title=\\normalsize\mbox{$\sigma="+s+"$},\n")
				f.write(" xmode=log,enlarge x limits=0.025,\n")
				if j == 0:
					f.write(" ylabel="+label+",\n")
				if j == 1:
					f.write(" xlabel=Number of elements \\footnotesize(logarithmic scale),"+\
					        "x label style={at={(axis description cs:0.5,-0.05)},anchor=north},\n")
					f.write(" yticklabels={},\n")
					f.write(" legend style={at={(0.55,1.45)},legend style={text width=4em},"+\
						    "legend style={draw=none},anchor=north,legend columns=-1},\n")
				if j == 2: f.write(" axis y line*=right,\n")
				else:	   f.write(" axis y line*=left,\n")
				f.write(" ymin="+str(ymin)+",ymax="+str(ymax)+",\n")
				f.write(" tick align=outside,\n")# axis x line*=bottom,\n")
				f.write(" x axis line style = ultra thick,y axis line style={white},\n")
				f.write(" xtick={2000,10000,50000},xticklabels={2000,10000,50000},\n")
				f.write(" minor xtick={18000,26000,34000,42000,58000,66000},")
				f.write(" every y tick/.style={white},ymajorgrids,grid style={gray!30,thick}]\n")
				# f.write("\t axis background/.style={fill=gray!50!white}")
				if j == 1:
					for l,d in list(enumerate(_d.keys()))[1:]:
						f.write("\\addlegendimage{line width=3pt,mark=none,"+_c4[l-1]+"}\n")
						f.write("\\addlegendentry{$D="+d+"$}\n")
				for l,d in list(enumerate(_d.keys()))[1:]:
					for m in range(2,3):
						char = "t" if m == 1 else "c"
						f.write("\\addplot[very thick,mark=none,smooth,")
						if m == 1: f.write("dashed,forget plot,")
						f.write("color="+_c4[l-1]+"] table[] {\n")
						for k,r in enumerate(_r.keys()):
							f.write("\t"+str(_tr[i][j][k][l][m])+" "+str(array[i][j][k][l][m])+"\n")
						f.write("};\n")
				f.write("!\\end{axis}\n\\end{scope}\n")
			f.write("\\end{tikzpicture}\n\\end{center}\n")
			if name == "_pr":
				f.write("\\caption{Element preservation results using mesh remodelling - "+ _w[w][1] +" airfoil}")
			elif name == "_es":
				f.write("\\caption{Element surplus results on meshes from mesh remodelling - "+ _w[w][1] +" airfoil}")
			else:
				f.write("\\caption{Speed-up results by mesh remodelling - "+ _w[w][1] +" airfoil}")
			f.write("\\centering\\sffamily\\footnotesize\nAverage of thirty optimisation scenarios")
			f.write("\\end{figure}\n")
