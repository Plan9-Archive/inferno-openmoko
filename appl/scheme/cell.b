implement SCell;

include "sys.m";
sys: Sys;

include "bufio.m";
bufio: Bufio;
Iobuf: import bufio;

include "cell.m";

init(s: Sys)
{
	sys = s;
}

lcar(args: ref Cell): ref Cell
{
	if(args == nil)
		return nil;
	pick x := args {
	Link =>
		if(x.next == nil)
			return nil;
		return x.next.car;
	}
	return nil;
}

lcdr(args: ref Cell): ref Cell
{
	if(args == nil)
		return nil;
	pick x := args {
	Link =>
		if(x.next == nil)
			return nil;
		return x.next.cdr;
	}
	return nil;
}

lcons(car: ref Cell, cdr: ref Cell) : ref Cell
{
	return ref Cell.Link(ref Pair(car, cdr));
}

ldefine(sym: string, exp: ref Cell, envlist: list of ref Env):
	(ref Cell, list of ref Env)
{
	ilk: int;
	f: ref fn(args: ref Cell): ref Cell;

	if(exp == nil)
		return (ref Cell.Link(nil), envlist);
	if(sym == "") {
		error("empty string for variable in define\n");
		return (ref Cell.Link(nil), envlist);
	}
	ilk = Variable;
	f = nil;
	if(exp != nil) {
		pick q := exp {
		Symbol =>
			if(q.env != nil) {
				ilk = q.env.ilk;
				f = q.env.handler;
			}
		}
	}
	e := ref Env(sym, ilk, exp, f);
	envlist = e :: envlist;
	return (ref Cell.Symbol(sym, e), envlist);
}

leqp(x1, x2: ref Cell): int
{
	pick y1 := x1 {
	Boolean =>
		pick y2 := x2 {
		Boolean =>
			if(y1.b == y2.b)
				return 1;
		}
	Symbol =>
		pick y2 := x2 {
		Symbol =>
			if(y1.sym == y2.sym)
				return 1;
		}
	String =>
		pick y2 := x2 {
		String =>
			if(y1.str == y2.str)
				return 1;
		}
	Char =>
		pick y2 := x2 {
		Char =>
			if(y1.c == y2.c)
				return 1;
		}
	Number =>
		pick y2 := x2 {
		Number =>
			if((y1.ilk ^ y2.ilk) & Exact) 
				return 0;
			else if(y1.ilk & y2.ilk & Exact) {
				if(y1.i == y2.i && y1.j == y2.j)
					return 1;
			}
			else {
				if(y1.r == y2.r)
					return 1;
			}
		}
	Link =>
		pick y2 := x2 {
		Link =>
			if(y1.next == nil && y2.next == nil)
				return 1;
			if(y1.next != nil && y2.next != nil
					&& y1.next.car == y2.next.car
					&& y1.next.cdr == y2.next.cdr)
				return 1;
		}
	Lambda =>
		pick y2 := x2 {
		Lambda =>
			if(y1.formals == y2.formals
					&& y1.exp_list == y2.exp_list)
				return 1;
		}
	Vector =>
		pick y2 := x2 {
		Vector =>
			if(len y1.v != len y2.v)
				return 0;
			for(i := 0; i < len y1.v; ++i)
				if(leqp(y1.v[i], y2.v[i]) == 0)
					return 0;
			return 1;
		}
	Port =>
		pick y2 := x2 {
		Port =>
			if(y1.p == y2.p && y1.dir == y2.dir)
				return 1;
		}
	}
	return 0;
}

leqvp(x1, x2: ref Cell): int
{
	pick y1 := x1 {
	Boolean =>
		pick y2 := x2 {
		Boolean =>
			if(y1.b == y2.b)
				return 1;
		}
	Symbol =>
		pick y2 := x2 {
		Symbol =>
			if(y1.sym == y2.sym)
				return 1;
		}
	String =>
		pick y2 := x2 {
		String =>
			if(y1.str == y2.str)
				return 1;
		}
	Char =>
		pick y2 := x2 {
		Char =>
			if(y1.c == y2.c)
				return 1;
		}
	Number =>
		pick y2 := x2 {
		Number =>
			if((y1.ilk ^ y2.ilk) & Exact) 
				return 0;
			else if(y1.ilk & y2.ilk & Exact) {
				if(y1.i == y2.i && y1.j == y2.j)
					return 1;
			}
			else {
				if(y1.r == y2.r)
					return 1;
			}
		}
	Link =>
		pick y2 := x2 {
		Link =>
			if(y1.next == nil && y2.next == nil)
				return 1;
			if(y1.next != nil && y2.next != nil
					&& y1.next.car == y2.next.car
					&& y1.next.cdr == y2.next.cdr)
				return 1;
		}
	Lambda =>
		pick y2 := x2 {
		Lambda =>
			if(y1.formals == y2.formals
					&& y1.exp_list == y2.exp_list)
				return 1;
		}
	Vector =>
		pick y2 := x2 {
		Vector =>
			if(len y1.v != len y2.v)
				return 0;
			for(i := 0; i < len y1.v; ++i)
				if(leqvp(y1.v[i], y2.v[i]) == 0)
					return 0;
			return 1;
		}
	Port =>
		pick y2 := x2 {
		Port =>
			if(y1.p == y2.p && y1.dir == y2.dir)
				return 1;
		}
	}
	return 0;
}

lappend(c1, c2: ref Cell): ref Cell
{
	if(c1 == nil || isnil(c1))
		return c2;
	return lcons(lcar(c1), lappend(lcdr(c1), c2));
}

isnil(l: ref Cell): int
{
	if(l == nil)
		return 0;
	pick x := l {
	Link =>
		if(x.next == nil)
			return 1;
	}
	return 0;
}

lookupsym(symbol: string): ref Env
{
	for(l := envstack; l != nil; l = tl l) {
		x := hd l;
		if(x.name == symbol)
			return x;
	}
	for(l = globalenv; l != nil; l = tl l) {
		x := hd l;
		if(x.name == symbol)
			return x;
	}
	return nil;
}

listappend(l1, l2: list of ref Env): list of ref Env
{
	if(l1 == nil)
		return l2;
	return hd l1 :: listappend(tl l1, l2);
}

error(s: string)
{
	sys->fprint(sys->fildes(2), "*** Error: %s", s);
}
