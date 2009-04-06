import sys, os
wd = os.getcwd()
for line in sys.stdin:
    line = line.strip()
    parts = line.split(':')
    if len(parts)>2:
        print '%s\%s(%s): %s' % (wd, parts[0], parts[1], ':'.join(parts[2:]))
    else:
        print line