implement InitShell;

include "sys.m";
	sys: Sys;

include "draw.m";


InitShell: module
{
	init:	fn(nil: ref Draw->Context, nil: list of string);
};

Command: module
{
	init:	fn(ctxt: ref Draw->Context, argv: list of string);
};

err(s: string)
{
	sys->fprint(sys->fildes(2), "bootinit: %s\n", s);
}

dobind(f, t: string, flags: int): int
{
	if(sys->bind(f, t, flags) < 0) {
		err(sys->sprint("can't bind %s on %s: %r", f, t));
		return 0;
	}
	return 1;
}

init(nil: ref Draw->Context, nil: list of string)
{
	shell := load Command "/dis/sh.dis";
	#shell2 := load Command "/dis/sh.dis";
#	wmshell := load Command "/dis/wm/wm.dis";
	wmshell := load Command "/dis/wm/sh.dis";

	sys = load Sys Sys->PATH;

	if(sys != nil)
		sys->print("init: starting shell\n");

#	ipavailable := 0;
#	if (dobind("#l", "/net", sys->MREPL) &&  # ethernet
#	    dobind("#I", "/net", sys->MAFTER))   # IP
#		ipavailable = 1;

	dobind("#p", "/prog", sys->MREPL);	# prog device
	dobind("#d", "/fd", Sys->MREPL);
#	dobind("#i", "/dev", sys->MREPL); 	# draw device
#	dobind("#t", "/dev", sys->MAFTER);	# serial line
	dobind("#c", "/dev", sys->MAFTER); 	# console device
#	dobind("#W","/dev",sys->MAFTER);	# Flash
#	dobind("#O", "/dev", sys->MAFTER);	# Modem
#	dobind("#T","/dev",sys->MAFTER);	# Touchscreen
	dobind("#m", "/dev", sys->MAFTER); 	# /dev/pointer


	shell->init(nil, "-n"::"/start"::nil);

	shell = nil; spawn wmshell->init(nil, nil);
#	spawn shell->init(nil, nil);
}
