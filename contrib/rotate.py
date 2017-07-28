import sys
# rotate stdin 90 deg. anticlockwise to stdout
# coded by WaReZ GodZ:
#    pizducha + cincila
#               at cool winter 1995
#     -* *- E n j o y     t h e      s c e n e   ** -- ** !
#
# additional tweaking by Kaptain Z in hot 2001

input=[]
for line in sys.stdin:
    input.append(line.rstrip())

maxline=max(map(lambda x: len(x),input))

for i in range(maxline-1,-1,-1):
    for j in range(len(input)):
        if len(input[j])>i:
            sys.stdout.write(input[j][i])
        else:
            sys.stdout.write(' ')
    print
