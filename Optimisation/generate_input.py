from subprocess import call
from sys import stdout

wings = ["boeing737", "clarky", "eppler376", "eppler545", "gottingen459",\
         "gottingen702", "naca2412", "naca63206", "rae2822"]
wings = ["eppler376"]
sigmas = ["0.01", "0.05", "0.25"]

for w in wings:
	for s in sigmas:
		direc = "output/"+w+"/"+s
		call("mkdir -p "+direc, shell=True)
		with open('cmaes_initials.par', 'r') as f:
			data = f.readlines()
		data[18] = "    "+s+"\n"
		with open('cmaes_initials.par', 'w') as f:
			f.writelines(data)
		for i in range(1,31):
			stdout.write("\r"+" "*30+"\r"+w+" "+s+" "+str(i))
			stdout.flush()
			call("./main input/"+w+".dat "+direc+"/"+str(i), shell = True)
print()
