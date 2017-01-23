with open("../../Code/output/all/mesh/rae2822/0.25/400_8/4/2/24", "r") as f:
	nv = int(f.readline())
	vs = [list(map(lambda x: float(x), f.readline().split())) for i in range(nv)]
	nt = int(f.readline())
	ts = [list(map(lambda x: int(x), f.readline().split())) for i in range(nt)]

lines = []
for t in ts:
	if (vs[t[0]][0] > -0.2 and vs[t[0]][0] < 1.2 and vs[t[0]][1] > -0.3 and vs[t[0]][1] < 0.3) or \
	   (vs[t[1]][0] > -0.2 and vs[t[1]][0] < 1.2 and vs[t[1]][1] > -0.3 and vs[t[1]][1] < 0.3) or \
	   (vs[t[2]][0] > -0.2 and vs[t[2]][0] < 1.2 and vs[t[2]][1] > -0.3 and vs[t[2]][1] < 0.3):
		lines.append((t[0],t[1]))
		lines.append((t[1],t[2]))
		lines.append((t[2],t[0]))
lines = list(set((a,b) if a <= b else (b,a) for a,b in lines))

with open("mesh.dat","w") as f:
	f.write("\n\n".join(["\n".join([" ".join([str(c) for c in vs[p]]) for p in l]) for l in lines]))
	
