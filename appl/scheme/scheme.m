Scheme: module
{
	PATH: con "/dis/scheme.dis";

	init: fn(nil: ref Draw->Context, nil: list of string);
	eval: fn(c: ref Cell): ref Cell;
	readcell: fn(b: ref Iobuf): ref Cell;
	printcell: fn(x: ref Cell, b: ref Iobuf, disp: int);
	scannum: fn(s: string, radix: int): ref Cell;
	reduce: fn(n, m: big): (big, big);
};

