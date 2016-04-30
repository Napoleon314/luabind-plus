import subprocess

p = subprocess.Popen('cmake --help', shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
i = 0
gen_start = False
for line in p.stdout.readlines():
	str_line = line.decode("utf-8")
	if not gen_start:
		if str_line.find('Generators') == 0:
			gen_start = True
	else:
		if str_line[0] == ' ' and str_line[1] == ' ' and str_line[2].isalpha():
			l = str_line.split('=')
			l[0] = l[0].strip()
			print(l)


	