from random import randint

filenames = ['ecoli/ecoli1_1e06.fna', 'ecoli/ecoli2_1e06.fna', 'ecoli/ecoli3_1e06.fna', 
             'ecoli/ecoli4_1e06.fna', 'ecoli/ecoli5_1e06.fna', 'ecoli/ecoli6_1e06.fna']
files = []
for name in filenames:
    files.append(open(name, 'r').readlines())

for i in range(3, 6):
    size = len(files[0]) - 1
    idx = randint(0, size - 10**(i-1))
    for name, file in zip(filenames, files):
        new_name = name[:16] + str(i) + name[17:]
        with open(new_name, 'w') as f:
            f.write(file[0][0:-1] + ' 1e0' + str(i) + '\n')
            f.write(''.join(file[idx:idx+10**(i-1)]))