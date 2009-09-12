implement BuiltIn;

include "sys.m";
sys: Sys;

include "math.m";
math: Math;

include "draw.m";

include "bufio.m";
bufio: Bufio;
Iobuf: import bufio;

include "string.m";
str: String;

include "cell.m";
cell: SCell;
Cell: import cell;
Pair: import cell;
Env: import cell;

include "scheme.m";
scheme: Scheme;
eval: import scheme;
readcell: import scheme;
printcell: import scheme;
scannum: import scheme;
reduce: import scheme;

stdin, stdout: ref Iobuf;

include "builtin.m";

init(sy: Sys, sch: Scheme, c: SCell, m: Math, st: String,
	b: Bufio, in: ref Iobuf, out: ref Iobuf)
{
	sys = sy;
	scheme = sch;
	cell = c;
	math = m;
	str = st;
	bufio = b;
	stdin = in;
	stdout = out;

	e := ref Env("+", cell->BuiltIn, nil, add) :: cell->envstack;
	e = ref Env("*", cell->BuiltIn, nil, mult) :: e;
	e = ref Env("-", cell->BuiltIn, nil, minus) :: e;
	e = ref Env("/", cell->BuiltIn, nil, divide) :: e;
	e = ref Env("=", cell->BuiltIn, nil, numequal) :: e;
	e = ref Env("<", cell->BuiltIn, nil, numless) :: e;
	e = ref Env(">", cell->BuiltIn, nil, numgreater) :: e;
	e = ref Env("<=", cell->BuiltIn, nil, numleq) :: e;
	e = ref Env(">=", cell->BuiltIn, nil, numgeq) :: e;
	e = ref Env("acos", cell->BuiltIn, nil, acos) :: e;
	e = ref Env("apply", cell->BuiltIn, nil, apply) :: e;
	e = ref Env("asin", cell->BuiltIn, nil, asin) :: e;
	e = ref Env("atan", cell->BuiltIn, nil, atan) :: e;
	e = ref Env("call-with-current-continuation",
		cell->BuiltIn, nil, callwcont) :: e;
	e = ref Env("call-with-values", cell->BuiltIn, nil, callwval) :: e;
	e = ref Env("car", cell->BuiltIn, nil, car) :: e;
	e = ref Env("cdr", cell->BuiltIn, nil, cdr) :: e;
	e = ref Env("ceiling", cell->BuiltIn, nil, ceiling) :: e;
	e = ref Env("char?", cell->BuiltIn, nil, charp) :: e;
	e = ref Env("char=?", cell->BuiltIn, nil, chareqp) :: e;
	e = ref Env("char<?", cell->BuiltIn, nil, charltp) :: e;
	e = ref Env("char>?", cell->BuiltIn, nil, chargtp) :: e;
	e = ref Env("char<=?", cell->BuiltIn, nil, charlep) :: e;
	e = ref Env("char>=?", cell->BuiltIn, nil, chargep) :: e;
	e = ref Env("char->integer", cell->BuiltIn, nil, char2int) :: e;
	e = ref Env("close-input-port", cell->BuiltIn, nil, closeinport) :: e;
	e = ref Env("close-output-port", cell->BuiltIn, nil, closeoutport) :: e;
	e = ref Env("complex?", cell->BuiltIn, nil, complexp) :: e;
	e = ref Env("cons", cell->BuiltIn, nil, cons) :: e;
	e = ref Env("cos", cell->BuiltIn, nil, cos) :: e;
	e = ref Env("current-input-port", cell->BuiltIn, nil, curinport) :: e;
	e = ref Env("current-output-port", cell->BuiltIn, nil, curoutport) :: e;
	e = ref Env("denominator", cell->BuiltIn, nil, denominator) :: e;
	e = ref Env("display", cell->BuiltIn, nil, display) :: e;
	e = ref Env("dynamic-wind", cell->BuiltIn, nil, dynwind) :: e;
	e = ref Env("eof-object?", cell->BuiltIn, nil, eofp) :: e;
	e = ref Env("eq?", cell->BuiltIn, nil, eqp) :: e;
	e = ref Env("eqv?", cell->BuiltIn, nil, eqvp) :: e;
	e = ref Env("eval", cell->BuiltIn, nil, leval) :: e;
	e = ref Env("exact?", cell->BuiltIn, nil, exactp) :: e;
	e = ref Env("exact->inexact", cell->BuiltIn, nil, extoinex) :: e;
	e = ref Env("exp", cell->BuiltIn, nil, exp) :: e;
	e = ref Env("expt", cell->BuiltIn, nil, expt) :: e;
	e = ref Env("floor", cell->BuiltIn, nil, floor) :: e;
	e = ref Env("inexact?", cell->BuiltIn, nil, inexactp) :: e;
	e = ref Env("inexact->exact", cell->BuiltIn, nil, inextoex) :: e;
	e = ref Env("input-port?", cell->BuiltIn,nil, inportp) :: e;
	e = ref Env("integer?", cell->BuiltIn, nil, integerp) :: e;
	e = ref Env("integer->char", cell->BuiltIn, nil, int2char) :: e;
	e = ref Env("list", cell->BuiltIn, nil, llist) :: e;
	e = ref Env("load", cell->BuiltIn, nil, lload) :: e;
	e = ref Env("log", cell->BuiltIn, nil, log) :: e;
	e = ref Env("make-string", cell->BuiltIn, nil, makestring) :: e;
	e = ref Env("make-vector", cell->BuiltIn, nil, makevector) :: e;
	e = ref Env("modulo", cell->BuiltIn, nil, modulo) :: e;
	e = ref Env("null-environment", cell->BuiltIn, nil, nullenv) :: e;
	e = ref Env("number?", cell->BuiltIn, nil, numberp) :: e;
	e = ref Env("number->string", cell->BuiltIn, nil, numtostr) :: e;
	e = ref Env("numerator", cell->BuiltIn, nil, numerator) :: e;
	e = ref Env("open-input-file", cell->BuiltIn, nil, openinfile) :: e;
	e = ref Env("open-output-file", cell->BuiltIn, nil, openoutfile) :: e;
	e = ref Env("output-port?", cell->BuiltIn, nil, outportp) :: e;
	e = ref Env("pair?", cell->BuiltIn, nil, pairp) :: e;
	e = ref Env("peek-char", cell->BuiltIn, nil, peekchar) :: e;
	e = ref Env("procedure?", cell->BuiltIn, nil, procedurep) :: e;
	e = ref Env("quit", cell->BuiltIn, nil, quit) :: e;
	e = ref Env("quotient", cell->BuiltIn, nil, quotient) :: e;
	e = ref Env("rational?", cell->BuiltIn, nil, rationalp) :: e;
	e = ref Env("read", cell->BuiltIn, nil, lread) :: e;
	e = ref Env("read-char", cell->BuiltIn, nil, preadchar) :: e;
	e = ref Env("real?", cell->BuiltIn, nil, realp) :: e;
	e = ref Env("remainder", cell->BuiltIn, nil, remainder) :: e;
	e = ref Env("round", cell->BuiltIn, nil, round) :: e;
	e = ref Env("scheme-report-environment",
		cell->BuiltIn, nil, schrepenv) :: e;
	e = ref Env("set-car!", cell->BuiltIn, nil, setcar) :: e;
	e = ref Env("set-cdr!", cell->BuiltIn, nil, setcdr) :: e;
	e = ref Env("sin", cell->BuiltIn, nil, sin) :: e;
	e = ref Env("sqrt", cell->BuiltIn, nil, sqrt) :: e;
	e = ref Env("string?", cell->BuiltIn, nil, stringp) :: e;
	e = ref Env("string-length", cell->BuiltIn, nil, stringlen) :: e;
	e = ref Env("string->number", cell->BuiltIn, nil, strtonum) :: e;
	e = ref Env("string-ref", cell->BuiltIn, nil, stringref) :: e;
	e = ref Env("string-set!", cell->BuiltIn, nil, stringset) :: e;
	e = ref Env("string=?", cell->BuiltIn, nil, stringeq) :: e;
	e = ref Env("string-ci=?", cell->BuiltIn, nil, stringcieq) :: e;
	e = ref Env("string<?", cell->BuiltIn, nil, stringlt) :: e;
	e = ref Env("string>?", cell->BuiltIn, nil, stringgt) :: e;
	e = ref Env("string<=?", cell->BuiltIn, nil, stringle) :: e;
	e = ref Env("string>=?", cell->BuiltIn, nil, stringge) :: e;
	e = ref Env("string-ci<?", cell->BuiltIn, nil, stringcilt) :: e;
	e = ref Env("string-ci>?", cell->BuiltIn, nil, stringcigt) :: e;
	e = ref Env("string-ci<=?", cell->BuiltIn, nil, stringcile) :: e;
	e = ref Env("string-ci>=?", cell->BuiltIn, nil, stringcige) :: e;
	e = ref Env("substring", cell->BuiltIn, nil, substring) :: e;
	e = ref Env("string-append", cell->BuiltIn, nil, stringappend) :: e;
	e = ref Env("string-copy", cell->BuiltIn, nil, stringcopy) :: e;
	e = ref Env("string-fill!", cell->BuiltIn, nil, stringfill) :: e;
	e = ref Env("string->symbol", cell->BuiltIn, nil, str2sym) :: e;
	e = ref Env("symbol?", cell->BuiltIn, nil, symbolp) :: e;
	e = ref Env("symbol->string", cell->BuiltIn, nil, sym2str) :: e;
	e = ref Env("tan", cell->BuiltIn, nil, tan) :: e;
	e = ref Env("truncate", cell->BuiltIn, nil, truncate) :: e;
	e = ref Env("values", cell->BuiltIn, nil, values) :: e;
	e = ref Env("vector?", cell->BuiltIn, nil, vectorp) :: e;
	e = ref Env("vector-length", cell->BuiltIn, nil, vectorlen) :: e;
	e = ref Env("vector-ref", cell->BuiltIn, nil, vectorref) :: e;
	e = ref Env("vector-set!", cell->BuiltIn, nil, vectorset) :: e;
	e = ref Env("write", cell->BuiltIn, nil, lwrite) :: e;
	e = ref Env("write-char", cell->BuiltIn, nil, writechar) :: e;
	l := e;
	while(l != cell->nullenvironment) {
		x := hd l;
		if(x.ilk == cell->BuiltIn || x.ilk == cell->SpecialForm)
			x.val = ref Cell.Symbol(x.name, x);
		l = tl l;
	}
	cell->baseenv = e;
	cell->globalenv = e;
	cell->envstack = nil;
}

acos(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in acos\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->acos(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	* =>
		cell->error("non-numeric argument to acos\n");
		return nil;
	}
	return ref Cell.Number(big 0, big 1, 0.0, 0);
}

add(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil)
		return ref Cell.Number(
			big 0, big 1, 0.0, cell->Integer|cell->Exact);
	pick y := x {
	Number =>
		if(cell->isnil(l))
			return ref Cell.Number(y.i, y.j, y.r, y.ilk);
		r := add(l);
		pick z := r {
		Number =>
			if(y.ilk & z.ilk & cell->Exact) {
				sn, sd: big;
				if(y.j == z.j) {
					sn = y.i + z.i;
					sd = y.j;
				}
				else {
					sn = y.i * z.j + z.i * y.j;
					sd = y.j * z.j;
				}
				if(sd != big 1)
					(sn, sd) = reduce(sn, sd);
				return ref Cell.Number(
					sn, sd, real sn / real sd, cell->Exact);
			}
			else {
				s := y.r + z.r;
				return ref Cell.Number(
					big s, big 1, s, 0);
			}
		* =>
			cell->error("non-numeric argument to +\n");
			return nil;
		}
	* =>
		cell->error("non-numeric argument to +\n");
		return nil;
	}
	return ref Cell.Number(big 0, big 1, 0.0, cell->Integer|cell->Exact);
}

makequoted(x: ref Cell): ref Pair
{
	p1 := ref Pair(x, ref Cell.Link(nil));
	p2 := ref Pair(ref Cell.Symbol("quote", cell->lookupsym("quote")),
		ref Cell.Link(p1));
	return ref Pair(ref Cell.Link(p2), ref Cell.Link(nil));
}					

apply(args: ref Cell): ref Cell
{
	if(args == nil || cell->isnil(args)) {
		cell->error("wrong number of arguments in apply\n");
		return nil;
	}
	newargs := ref Cell.Link(ref Pair(cell->lcar(args), ref Cell.Link(nil)));
	np := cell->lcdr(newargs);
	oldp := cell->lcdr(args);
	firstpart := 1;
	while(1) {
		if(oldp == nil || cell->isnil(oldp))
			break;
		t := cell->lcdr(oldp);
		if(t == nil || cell->isnil(t)) {
			if(firstpart == 1) {
				oldp = cell->lcar(oldp);
				firstpart = 0;
			}
			else {
				pick npl := np {
				Link =>
					npl.next = makequoted(cell->lcar(oldp));
				}
				break;
			}
		}
		else {
			pick npl := np {
			Link =>
				npl.next = makequoted(cell->lcar(oldp));
			}
			np = cell->lcdr(np);
			oldp = t;
		}
	}
	return eval(newargs);
}

asin(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in asin\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->asin(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	* =>
		cell->error("non-numeric argument to asin\n");
		return nil;
	}
	return ref Cell.Number(big 0, big 1, 0.0, 0);
}

atan(args: ref Cell): ref Cell
{
	n: real;

	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in atan\n");
		return nil;
	}
	pick y := x {
	Number =>
		n = y.r;
	* =>
		cell->error("non-numeric argument to atan\n");
		return nil;
	}
	l := cell->lcdr(args);
	if(l == nil || cell->isnil(l)) {
		m := math->atan(n);
		return ref Cell.Number(big m, big 1, m, 0);
	}
	z := cell->lcar(l);
	pick zn := z {
	Number =>
		m := math->atan2(n, zn.r);
		return ref Cell.Number(big m, big 1, m, 0);
	* =>
		cell->error("non-numeric argument to atan\n");
		return nil;
	}
	return nil;
}

callwcont(nil: ref Cell): ref Cell
{
	return ref Cell.Link(nil);
}

callwval(nil: ref Cell): ref Cell
{
	return ref Cell.Link(nil);
}

car(args: ref Cell): ref Cell
{
	l := cell->lcar(args);
	if(l == nil) {
		cell->error("non-pair argument to car\n");
		return nil;
	}
	return cell->lcar(l);
}

cdr(args: ref Cell): ref Cell
{
	l := cell->lcar(args);
	if(l == nil) {
		cell->error("non-pair argument to cdr\n");
		return nil;
	}
	return cell->lcdr(l);
}

ceiling(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in ceiling\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->ceil(y.r);
		return ref Cell.Number(big n, big 1, n, cell->Exact);
	* =>
		cell->error("non-numeric argument to ceiling\n");
	}
	return nil;
}

charp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in char?\n");
		return nil;
	}
	pick y := x {
	Char =>
		return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

chareqp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to char=?\n");
		return nil;
	}
	pick x1 := x {
	Char =>
		pick y1 := y {
		Char =>
			if(x1.c == y1.c)
				return ref Cell.Boolean(1);
		* =>
			cell->error("non-character argument to char=?\n");
			return nil;
		}
	* =>
		cell->error("non-character argument to char=?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

charltp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to char<?\n");
		return nil;
	}
	pick x1 := x {
	Char =>
		pick y1 := y {
		Char =>
			if(x1.c < y1.c)
				return ref Cell.Boolean(1);
		* =>
			cell->error("non-character argument to char<?\n");
			return nil;
		}
	* =>
		cell->error("non-character argument to char<?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

chargtp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to char>?\n");
		return nil;
	}
	pick x1 := x {
	Char =>
		pick y1 := y {
		Char =>
			if(x1.c > y1.c)
				return ref Cell.Boolean(1);
		* =>
			cell->error("non-character argument to char>?\n");
			return nil;
		}
	* =>
		cell->error("non-character argument to char>?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

charlep(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to char<=?\n");
		return nil;
	}
	pick x1 := x {
	Char =>
		pick y1 := y {
		Char =>
			if(x1.c <= y1.c)
				return ref Cell.Boolean(1);
		* =>
			cell->error("non-character argument to char<=?\n");
			return nil;
		}
	* =>
		cell->error("non-character argument to char<=?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

chargep(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to char>=?\n");
		return nil;
	}
	pick x1 := x {
	Char =>
		pick y1 := y {
		Char =>
			if(x1.c >= y1.c)
				return ref Cell.Boolean(1);
		* =>
			cell->error("non-character argument to char>=?\n");
			return nil;
		}
	* =>
		cell->error("non-character argument to char>=?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

char2int(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in char->integer\n");
		return nil;
	}
	pick y := x {
	Char =>
		return ref Cell.Number(
			big y.c, big 1, real y.c, cell->Integer|cell->Exact);
	}
	cell->error("non-character argument to char->integer\n");
	return nil;
}

closeinport(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in close-*-port\n");
		return nil;
	}
	pick y := x {
	Port =>
		y.p = nil;
		y.dir = -1;
	* =>
		cell->error("non-port argument to close-*-port\n");
	}
	return nil;
}

closeoutport(args: ref Cell): ref Cell
{
	return closeinport(args);
}

complexp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in complex?\n");
		return nil;
	}
	pick y := x {
	Number =>
		return ref Cell.Boolean(1);
	}
	cell->error("non-numeric argument to complex?\n");
	return nil;
}

cons(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in cons\n");
		return nil;
	}
	return cell->lcons(x, cell->lcar(l));
}

cos(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in cos\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->cos(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	}
	cell->error("non-numeric argument to cos\n");
	return nil;
}

curinport(nil: ref Cell): ref Cell
{
	return ref Cell.Port(stdin, Bufio->OREAD);
}

curoutport(nil: ref Cell): ref Cell
{
	return ref Cell.Port(stdout, Bufio->OWRITE);
}

denominator(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in denominator\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(y.ilk & cell->Exact) {
			return ref Cell.Number(y.j, big 1, real y.j, cell->Exact);
		}
	* =>
		cell->error("non-numeric argument to denominator\n");
		return nil;
	}
	return ref Cell.Number(big 0, big 1, 0.0, cell->Exact);
}

display(args: ref Cell): ref Cell
{
	port := stdout;
	x := cell->lcar(args);
	if(x == nil)
		return nil;
	l := cell->lcdr(args);
	if(l != nil && !(cell->isnil(l))) {
		p := cell->lcar(l);
		pick q := p {
		Port =>
			if(q.dir != -1)
				port = q.p;
			else
				return nil;
		}
	}
	printcell(x, port, 1);
	return x;
}

divide(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil) {
		cell->error("wrong number of arguments in /\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(cell->isnil(l))
			return ref Cell.Number(y.j, y.i, 1.0 / y.r, y.ilk);
		r := mult(l);
		pick z := r {
		Number =>
			if(z.ilk & y.ilk & cell->Exact) {
				dn := y.i * z.j;
				dd := y.j * z.i;
				if(dd != big 1)
					(dn, dd) = reduce(dn, dd);
				return ref Cell.Number(
					dn, dd, real dn / real dd, cell->Exact);
			}
			else {
				quot := y.r / z.r;
				return ref Cell.Number(
					big quot, big 1, quot, cell->Real);
			}
		}
	}
	cell->error("non-numeric argument to /\n");
	return nil;
}

dynwind(nil:ref Cell): ref Cell
{
	return ref Cell.Link(nil);
}

eofp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in eof?\n");
		return nil;
	}
	pick y := x {
	Char =>
		if(y.c == Bufio->EOF)
			return ref Cell.Boolean(1);
	* =>
		cell->error("non-character argument to eof?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

eqp(args: ref Cell): ref Cell
{
	x1 := cell->lcar(args);
	l := cell->lcdr(args);
	x2 := cell->lcar(l);
	if(x1 == nil || x2 == nil|| l== nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in eq?\n");
		return nil;
	}
	return ref Cell.Boolean(cell->leqp(x1, x2));
}

eqvp(args: ref Cell): ref Cell
{
	x1 := cell->lcar(args);
	l := cell->lcdr(args);
	x2 := cell->lcar(l);
	if(x1 == nil || x2 == nil || l== nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in eqv?\n");
		return nil;
	}
	return ref Cell.Boolean(cell->leqvp(x1, x2));
}

leval(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil || cell->isnil(l))
		return nil;
	y := cell->lcar(l);
	if(y == nil) {
		cell->error("wrong number of arguments in eval\n");
		return nil;
	}
	pick yn := y {
	Environment =>
		saveenv := cell->envstack;
		cell->envstack = yn.env;
		c := eval(x);
		cell->envstack = saveenv;
		return c;
	* =>
		cell->error("non-environment argument to eval\n");
		return nil;
	}
	return nil;
}

exactp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in exact?\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(y.ilk & cell->Exact)
			return ref Cell.Boolean(1);
	* =>
		cell->error("non-numeric argument to exact?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

extoinex(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in exact->inexact\n");
		return nil;
	}
	pick y := x {
	Number =>
		return ref Cell.Number(y.i, y.j, y.r, 0);
	}
	cell->error("non-numeric argument to exact->inexact\n");
	return ref Cell.Number(big 0, big 1, 0.0, cell->Exact);
}

exp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in exp\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->exp(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	}
	cell->error("non-numeric argument to exp\n");
	return nil;
}

expt(args: ref Cell): ref Cell
{
	z2: real;
	zl: int;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to expt\n");
		return nil;
	}
	pick yn := y {
	Number =>
		z2 = yn.r;
		zl = yn.ilk;
	* =>
		cell->error("non-numeric argument to expt\n");
		return nil;
	}
	pick xn := x {
	Number =>
		n := math->pow(xn.r, z2);
		return ref Cell.Number(big n, big 1, n, xn.ilk & zl);
	* =>
		cell->error("non-numeric argument to expt\n");
		return nil;
	}
	return nil;
}

floor(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in floor\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->floor(y.r);
		return ref Cell.Number(big n, big 1, n, cell->Exact);
	}
	cell->error("non-numeric argument to floor\n");
	return nil;
}

inexactp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in inexact?\n");
		return nil;
	}
	pick y := x {
	Number =>
		if((y.ilk & cell->Exact) == 0)
			return ref Cell.Boolean(1);
	* =>
		cell->error("non-numeric argument to inexact?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

inextoex(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in inexact->exact\n");
		return nil;
	}
	pick y := x {
	Number =>
		return ref Cell.Number(y.i, y.j, y.r, cell->Exact);
	}
	cell->error("non-numeric argument to inexact->exact\n");
	return nil;
}

inportp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in inport?\n");
		return nil;
	}
	pick y := x {
	Port =>
		if(y.dir == Bufio->OREAD)
			return ref Cell.Boolean(1);
	* =>
		cell->error("non-numeric argument to inport?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

integerp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to integer?\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(y.ilk & cell->Exact) {
			if(y.j == big 1)
				return ref Cell.Boolean(1);
		}
		else {
			n := math->rint(y.r);
			if(real n == y.r)
				return ref Cell.Boolean(1);
		}
	}
	return ref Cell.Boolean(0);
}

int2char(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in integer->char\n");
		return nil;
	}
	pick y := x {
	Number =>
		return ref Cell.Char(int y.i);
	}
	cell->error("non-numeric argument to integer->char\n");
	return nil;
}

llist(args: ref Cell): ref Cell
{
	return args;
}

lload(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to load\n");
		return nil;
	}
	pick y := x {
	String =>
		b := bufio->open(y.str, Bufio->OREAD);
		if(b == nil) {
			cell->error(sys->sprint("can't load%s : %r\n", y.str));
			return nil;
		}
		while(1) {
			c := readcell(b);
			if(c == nil)
				break;
			eval(c);
		}
		b = nil;
		return ref Cell.String(y.str);
	}
	cell->error("non-string argument to load\n");
	return nil;
}

log(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in log\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->log(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	}
	cell->error("non-numeric argument to log\n");
	return nil;
}

makestring(args: ref Cell): ref Cell
{
	c := ' ';
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in make-string\n");
		return nil;
	}
	l := cell->lcdr(args);
	if(l != nil && !(cell->isnil(l))) {
		y := cell->lcar(l);
		if(y != nil) {
			pick y1 := y {
			Char =>
				c = y1.c;
			}
		}
	}
	pick x1 := x {
	Number =>
		s: string;
		for(i := 0; big i < x1.i; ++i)
			s[i] = c;
		return ref Cell.String(s);
	}
	cell->error("non-numeric argument to make-string\n");
	return nil;
}

makevector(args: ref Cell): ref Cell
{
	v: array of ref Cell;
	k: int;

	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in make-vector\n");
		return nil;
	}
	pick x1 := x {
	Number =>
		k = int x1.i;
		v = array[k] of ref Cell;
	* =>
		cell->error("non-numeric argument to make-vector\n");
		return nil;
	}
	l := cell->lcdr(args);
	if(l != nil && !(cell->isnil(l))) {
		y := cell->lcar(l);
		for(i := 0; i < k; ++i)
			v[i] = y;
	}
	return ref Cell.Vector(v);
}

minus(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil)
		return ref Cell.Number(
			big 0, big 1, 0.0, cell->Integer|cell->Exact);
	pick y := x {
	Number =>
		if(cell->isnil(l))
			return ref Cell.Number(-y.i, y.j, -y.r, y.ilk);
		r := add(l);
		pick z := r {
		Number =>
			if(z.ilk & y.ilk & cell->Exact) {
				dn, dd: big;
				if(y.j == z.j) {
					dn = y.i - z.i;
					dd = y.j;
				}
				else {
					dn = y.i * z.j - z.i * y.j;
					dd = y.j * z.j;
				}
				if(dd != big 1)
					(dn, dd) = reduce(dn, dd);
				return ref Cell.Number(
					dn, dd, real dn / real dd, cell->Exact);
			}
			else {
				diff := y.r - z.r;
				return ref Cell.Number(
					big diff, big 1, diff, cell->Real);
			}
		* =>
			cell->error("non-numeric argument to -\n");
			return nil;
		}
	* =>
		cell->error("non-numeric argument to -\n");
		return nil;
	}
	return ref Cell.Number(
		big 0, big 1, 0.0, cell->Integer|cell->Exact);
}

modulo(args: ref Cell): ref Cell
{
	numer, denom: big;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in modulo\n");
		return nil;
	}
	pick z := x {
	Number =>
		numer = z.i;
	* =>
		cell->error("non-numeric argument to modulo\n");
		return nil;
	}
	pick z := y {
	Number =>
		denom = z.i;
	* =>
		cell->error("non-numeric argument to modulo\n");
		return nil;
	}
	mod := numer % denom;
	if(denom > big 0 && mod < big 0)
		mod += denom;
	else if(denom < big 0 && mod > big 0)
		mod += denom;
	return ref Cell.Number(
		mod, big 1, real mod, cell->Integer|cell->Exact);
}

mult(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil)
		return ref Cell.Number(
			big 1, big 1, 1.0, cell->Integer|cell->Exact);
	pick y := x {
	Number =>
		if(cell->isnil(l))
			return ref Cell.Number(y.i, y.j, y.r, y.ilk);
		r := mult(l);
		pick z := r {
		Number =>
			if(y.ilk & z.ilk & cell->Exact) {
				pn := y.i * z.i;
				pd := y.j * z.j;
				if(pd != big 1)
					(pn, pd) = reduce(pn, pd);
				return ref Cell.Number(
					pn, pd, real pn / real pd, cell->Exact);
			}
			else {
				prod := y.r * z.r;
				return ref Cell.Number(big prod, big 1, prod, 0);
			}
		* =>
			cell->error("non-numeric argument to +\n");
			return nil;
		}
	* =>
		cell->error("non-numeric argument to +\n");
		return nil;
	}
	return ref Cell.Number(
		big 0, big 1, 0.0, cell->Integer|cell->Exact);
}

nullenv(args: ref Cell): ref Cell
{
	if(args == nil || cell->isnil(args))
		return nil;
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in null-environment\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(y.i == big 5 && y.j == big 1)
			return ref Cell.Environment(cell->nullenvironment);
	* =>
		cell->error("non-numeric argument to null-environment\n");
	}
	cell->error("unsupported environment version\n");
	return nil;
}

numberp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to number?\n");
		return nil;
	}
	pick y := x {
	Number =>
		return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

numequal(args: ref Cell): ref Cell
{
	y: ref Cell;
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil) {
		cell->error("wrong number of arguments in =\n");
		return nil;
	}
	while(l != nil && !(cell->isnil(l))) {
		pick xn := x {
		Number =>
			y = cell->lcar(l);
			pick yn := y {
			Number =>
				if(xn.ilk & yn.ilk & cell->Exact) {
					if(xn.i * yn.j != yn.i * xn.j)
						return ref Cell.Boolean(0);
				}
				else if(xn.r != yn.r)
					return ref Cell.Boolean(0);
			* =>
				cell->error("non-numeric argument to =\n");
				return nil;
			}
		* =>
			cell->error("non-numeric argument to =\n");
			return nil;
		}
		l = cell->lcdr(l);
		x = y;
	}
	return ref Cell.Boolean(1);
}

numgeq(args: ref Cell): ref Cell
{
	y: ref Cell;
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil) {
		cell->error("wrong number of arguments in >=\n");
		return nil;
	}
	while(l != nil && !(cell->isnil(l))) {
		pick xn := x {
		Number =>
			y = cell->lcar(l);
			pick yn := y {
			Number =>
				if(xn.ilk & yn.ilk & cell->Exact) {
					if(xn.i * yn.j < yn.i * xn.j)
						return ref Cell.Boolean(0);
				}
				else if(xn.r < yn.r)
					return ref Cell.Boolean(0);
			* =>
				cell->error("non-numeric argument to >=\n");
				return nil;
			}
		* =>
			cell->error("non-numeric argument to >=\n");
			return nil;
		}
		l = cell->lcdr(l);
		x = y;
	}
	return ref Cell.Boolean(1);
}

numgreater(args: ref Cell): ref Cell
{
	y: ref Cell;
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil) {
		cell->error("wrong number of arguments in >\n");
		return nil;
	}
	while(l != nil && !(cell->isnil(l))) {
		pick xn := x {
		Number =>
			y = cell->lcar(l);
			pick yn := y {
			Number =>
				if(xn.ilk & yn.ilk & cell->Exact) {
					if(xn.i * yn.j <= yn.i * xn.j)
						return ref Cell.Boolean(0);
				}
				else if(xn.r <= yn.r)
					return ref Cell.Boolean(0);
			* =>
				cell->error("non-numeric argument to >\n");
				return nil;
			}
		* =>
			cell->error("non-numeric argument to >\n");
			return nil;
		}
		l = cell->lcdr(l);
		x = y;
	}
	return ref Cell.Boolean(1);
}

numless(args: ref Cell): ref Cell
{
	y: ref Cell;
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil) {
		cell->error("wrong number of arguments in <\n");
		return nil;
	}
	while(l != nil && !(cell->isnil(l))) {
		pick xn := x {
		Number =>
			y = cell->lcar(l);
			pick yn := y {
			Number =>
				if(xn.ilk & yn.ilk & cell->Exact) {
					if(xn.i * yn.j >= yn.i * xn.j)
						return ref Cell.Boolean(0);
				}
				else if(xn.r >= yn.r)
					return ref Cell.Boolean(0);
			* =>
				cell->error("non-numeric argument to <\n");
				return nil;
			}
		* =>
			cell->error("non-numeric argument to <\n");
			return nil;
		}
		l = cell->lcdr(l);
		x = y;
	}
	return ref Cell.Boolean(1);
}

numleq(args: ref Cell): ref Cell
{
	y: ref Cell;
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil) {
		cell->error("wrong number of arguments in <=\n");
		return nil;
	}
	while(l != nil && !(cell->isnil(l))) {
		pick xn := x {
		Number =>
			y = cell->lcar(l);
			pick yn := y {
			Number =>
				if(xn.ilk & yn.ilk & cell->Exact) {
					if(xn.i * yn.j > yn.i * xn.j)
						return ref Cell.Boolean(0);
				}
				else if(xn.r > yn.r)
					return ref Cell.Boolean(0);
			* =>
				cell->error("non-numeric argument to <=\n");
				return nil;
			}
		* =>
			cell->error("non-numeric argument to <=\n");
			return nil;
		}
		l = cell->lcdr(l);
		x = y;
	}
	return ref Cell.Boolean(1);
}

numtostr(args: ref Cell): ref Cell
{
	radix := 10;
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to number->string\n");
		return nil;
	}
	l := cell->lcdr(args);
	if(l != nil && !(cell->isnil(l))) {
		y := cell->lcar(l);
		pick yn := y {
		Number =>
			radix = int yn.i;
		}
	}
	pick xn := x {
	Number =>
		if((xn.ilk & cell->Exact) == 0)
			return ref Cell.String(sys->sprint("%.#g", xn.r));
		case radix {
		2 =>
			if(xn.j == big 1)
				return ref Cell.String(binstr(xn.i));
			else
				return ref Cell.String(sys->sprint("%s/%s",
					binstr(xn.i), binstr(xn.j)));
		8 =>	
			if(xn.j == big 1)
				return ref Cell.String(sys->sprint("%bo", xn.i));
			else
				return ref Cell.String(
					sys->sprint("%bo/%bo", xn.i, xn.j));
		16 =>
			if(xn.j == big 1)
				return ref Cell.String(sys->sprint("%bx", xn.i));
			else
				return ref Cell.String(
					sys->sprint("%bx/%bx", xn.i, xn.j));
		* =>
			if(xn.j == big 1)
				return ref Cell.String(sys->sprint("%bd", xn.i));
			else
				return ref Cell.String(
					sys->sprint("%bd/%bd", xn.i, xn.j));
		}
	* =>
		cell->error("non-numeric argument to number->string\n");
	}
	return nil;
}

numerator(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to numerator\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(y.ilk & cell->Exact) {
			if(y.i >= big 0)
				n := y.i;
			else
				n = -y.i;
			return ref Cell.Number(n, big 1, real n, cell->Exact);
		}
	* =>
		cell->error("non-numeric argument to numerator\n");
	}
	return nil;
}

openinfile(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to open-input-file\n");
		return nil;
	}
	pick y := x {
	String =>
		b := bufio->open(y.str, Bufio->OREAD);
		if(b == nil) {
			cell->error(sys->sprint("Cannot open %s: %r\n", y.str));
			return nil;
		}
		return ref Cell.Port(b, Bufio->OREAD);
	* =>
		cell->error("non-string argument to open-input-file\n");
	}
	return nil;
}

openoutfile(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to open-output-file\n");
		return nil;
	}
	pick y := x {
	String =>
		b := bufio->create(y.str, Bufio->OWRITE, 8r664);
		if(b == nil) {
			cell->error(sys->sprint("Cannot open %s: %r\n", y.str));
			return nil;
		}
		return ref Cell.Port(b, Bufio->OWRITE);
	* =>
		cell->error("non-string argument to open-output-file\n");
	}
	return nil;
}

outportp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments for output-port?\n");
		return nil;
	}
	pick y := x {
	Port =>
		if(y.dir == Bufio->OWRITE)
			return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

pairp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to pair?\n");
		return nil;
	}
	pick y := x {
	Link =>
		if(y.next != nil)
			return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

peekchar(args: ref Cell): ref Cell
{
	port: ref Iobuf;

	x := cell->lcar(args);
	if(x == nil)
		port = stdin;
	else {
		pick y := x {
		Port =>
			if(y.dir != -1)
				port = y.p;
			else {
				cell->error("non-open port in peek-char\n");
				return nil;
			}
		}
	}
	c := port.getc();
	port.ungetc();
	return ref Cell.Char(c);
}

procedurep(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to procedure?\n");
		return nil;
	}
	pick y := x {
	Lambda =>
		return ref Cell.Boolean(1);
	Symbol =>
		e := cell->lookupsym(y.sym);
		if(e.ilk == cell->BuiltIn)
			return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

quotient(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in quotient\n");
		return nil;
	}
	q := big 1;
	pick z := x {
	Number =>
		q = z.i;
	* =>
		cell->error("non-numeric argument to quotiend\n");
		return nil;
	}
	pick z := y {
	Number =>
		q /= z.i;
		return ref Cell.Number(
			q, big 1, real q, cell->Integer|cell->Exact);
	* =>
		cell->error("non-numeric argument to quotient\n");
		return nil;
	}
	return nil;
}

quit(nil: ref Cell): ref Cell
{
	exit;
}

rationalp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in rational?\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(y.ilk & cell->Exact)
			return ref Cell.Boolean(1);
	* =>
		cell->error("non-numeric argument to rational?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

lread(args: ref Cell): ref Cell
{
	port := stdin;
	x := cell->lcar(args);
	if(x != nil) {
		pick y := x {
		Port =>
			if(y.dir != -1)
				port = y.p;
			else {
				cell->error("non-open port in read\n");
				return nil;
			}
		}
	}
	return readcell(port);
}

preadchar(args: ref Cell): ref Cell
{
	port := stdin;
	x := cell->lcar(args);
	if(x != nil) {
		pick y := x {
		Port =>
			if(y.dir != -1)
				port = y.p;
			else {
				cell->error("non-open port in read-char\n");
				return nil;
			}
		}
	}
	c := port.getc();
	return ref Cell.Char(c);
}

realp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to real?\n");
		return nil;
	}
	pick y := x {
	Number =>
		return ref Cell.Boolean(1);
	* =>
		cell->error("non-numeric argument to real?\n");
		return nil;
	}
	return ref Cell.Boolean(0);
}

remainder(args: ref Cell): ref Cell
{
	numer, denom: big;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in remainder\n");
		return nil;
	}
	pick z := x {
	Number =>
		numer = z.i;
	* =>
		cell->error("non-numeric argument to remainder\n");
		return nil;
	}
	pick z := y {
	Number =>
		denom = z.i;
	* =>
		cell->error("non-numeric argument in remainder\n");
		return nil;
	}
	mod := numer % denom;
	if(numer > big 0 && mod < big 0)
		mod += denom;
	else if(numer < big 0 && mod > big 0)
		mod -= denom;
	return ref Cell.Number(
		mod, big 1, real mod, cell->Integer|cell->Exact);
}

round(args: ref Cell): ref Cell
{
	math->FPcontrol(math->RND_NR, math->RND_MASK);
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to round\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->rint(y.r);
		return ref Cell.Number(big n, big 1, n, cell->Exact);
	* =>
		cell->error("non-numeric argument to round\n");
	}
	return nil;
}

schrepenv(args: ref Cell): ref Cell
{
	if(args == nil || cell->isnil(args))
		return nil;
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in scheme-repeat-environment\n");
		return nil;
	}
	pick y := x {
	Number =>
		if(y.i == big 5 && y.j == big 1)
			return ref Cell.Environment(cell->reportenv);
	* =>
		cell->error("non-numeric argument to scheme-report-environment\n");
	}
	cell->error("unsupported version in scheme-report-environment\n");
	return nil;
}

setcar(args: ref Cell): ref Cell
{
	p := cell->lcar(args);
	l := cell->lcdr(args);
	o := cell->lcar(l);
	if(p == nil || o == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in set-car!\n");
		return nil;
	}
	pick x := p {
	Link =>
		x.next.car = o;
	* =>
		cell->error("non-pair argument to set-car!\n");
		return nil;
	}
	return p;
}

setcdr(args: ref Cell): ref Cell
{
	p := cell->lcar(args);
	l := cell->lcdr(args);
	o := cell->lcar(l);
	if(p == nil || o == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to set-cdr!\n");
		return nil;
	}
	pick x := p {
	Link =>
		x.next.cdr = o;
	* =>
		cell->error("non-pair argument to set->cdr!\n");
		return nil;
	}
	return nil;
}

sin(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in sin\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->sin(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	* =>
		cell->error("non-numeric argument to sin\n");
	}
	return nil;
}

sqrt(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in sqrt\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->sqrt(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	* =>
		cell->error("non-numeric argument to sqrt\n");
	}
	return nil;
}

stringp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to string?\n");
		return nil;
	}
	pick y := x {
	String =>
		return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

stringlen(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in string-length\n");
		return nil;
	}
	pick y := x {
	String =>
		z := len y.str;
		return ref Cell.Number(
			big z, big 1, real z, cell->Integer|cell->Exact);
	* =>
		cell->error("non-string argument to string-length\n");
	}
	return nil;
}

stringref(args: ref Cell): ref Cell
{
	i: int;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to string-ref\n");
		return nil;
	}
	pick y1 := y {
	Number =>
		i = int y1.i;
	* =>
		cell->error("non-numeric index in string-ref\n");
		return nil;
	}
	pick x1 := x {
	String =>
		return ref Cell.Char(x1.str[i]);
	* =>
		cell->error("non-string argument to string-ref\n");
	}
	return nil;
}

stringset(args: ref Cell): ref Cell
{
	i: int;
	c: int;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	l = cell->lcdr(l);
	z := cell->lcar(l);
	if(x == nil || y == nil || z == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to string-set!\n");
		return nil;
	}
	pick y1 := y {
	Number =>
		i = int y1.i;
	* =>
		cell->error("non-numeric index to string-set!\n");
		return nil;
	}
	pick z1 := z {
	Char =>
		c = z1.c;
	* =>
		cell->error("non-character fill in string-set!\n");
		return nil;
	}
	pick x1 := x {
	String =>
		x1.str[i] = c;
	* =>
		cell->error("non-string argument to string-set!\n");
		return nil;
	}
	return x;
}

getstrargs(args: ref Cell): (int, string, string)
{
	s1: string;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in string comparison\n");
		return (0, nil, nil);
	}
	pick x1 := x {
	String =>
		s1 = x1.str;
	* =>
		cell->error("non-string argument in string comparison\n");
		return (0,nil,nil);
	}
	pick y1 := y {
	String =>
		return (1, s1, y1.str);
	}
	cell->error("non-string argument in string comparison\n");
	return (0, nil, nil);
}

stringeq(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(s1 == s2)
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringcieq(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(str->toupper(s1) == str->toupper(s2))
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringlt(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(s1 < s2)
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringgt(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(s1 > s2)
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringle(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(s1 <= s2)
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringge(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(s1 >= s2)
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringcilt(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(str->toupper(s1) < str->toupper(s2))
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringcigt(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(str->toupper(s1) > str->toupper(s2))
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringcile(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(str->toupper(s1) <= str->toupper(s2))
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

stringcige(args: ref Cell): ref Cell
{
	(r, s1, s2) := getstrargs(args);
	if(!r)
		return nil;
	if(str->toupper(s1) >= str->toupper(s2))
		return ref Cell.Boolean(1);
	else
		return ref Cell.Boolean(0);
}

strtonum(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil) {
		cell->error("wrong number of arguments in string->number\n");
		return nil;
	}
	radix := 10;
	if(l != nil && !(cell->isnil(l))) {
		y := cell->lcar(l);
		if(y == nil) {
			cell->error("non-numeric radix in string->number\n");
			return nil;
		}
		pick yn := y {
		Number =>
			radix = int yn.i;
		* =>
			cell->error("non-numeric radix in string->number\n");
			return nil;
		}
	}
	pick xn := x {
	String =>
		return scannum(xn.str, radix);
	* =>
		cell->error("non-string argument to string->number\n");
	}
	return nil;
}

substring(args: ref Cell): ref Cell
{
	start, end: int;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	l = cell->lcdr(l);
	z := cell->lcar(l);
	if(x == nil || y == nil || z == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in substring\n");
		return nil;
	}
	pick y1 := y {
	Number =>
		start = int y1.i;
	* =>
		cell->error("non-numeric index in substring\n");
		return nil;
	}
	pick z1 := z {
	Number =>
		end = int z1.i;
	* =>
		cell->error("non-numeric index in substring\n");
		return nil;
	}
	pick x1 := x {
	String =>
		return ref Cell.String(x1.str[start:end]);
	* =>
		cell->error("non-string argument to substring\n");
	}
	return nil;
}

stringappend(args: ref Cell): ref Cell
{
	s := "";
	l := args;
	while(l != nil && !(cell->isnil(l))) {
		x := cell->lcar(l);
		if(x == nil)
			return nil;
		l = cell->lcdr(l);
		pick y := x {
		String =>
			s += y.str;
		* =>
			cell->error("non-string argument to string-append\n");
			return nil;
		}
	}
	return ref Cell.String(s);
}

stringcopy(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to string-copy\n");
		return nil;
	}
	pick y := x {
	String =>
		return ref Cell.String(y.str);
	* =>
		cell->error("non-string argument to string-copy\n");
	}
	return nil;
}

stringfill(args: ref Cell): ref Cell
{
	c: int;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to string-fill!\n");
		return nil;
	}
	pick y1 := y {
	Char =>
		c = y1.c;
	* =>
		cell->error("non-character fill in string-fill!\n");
		return nil;
	}
	pick x1 := x {
	String =>
		for(i := 0; i < len x1.str; ++i)
			x1.str[i] = c;
	* =>
		cell->error("non-string argument to string-fill!\n");
		return nil;
	}
	return x;
}

str2sym(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in string->symbol\n");
		return nil;
	}
	pick y := x {
	String =>
		return ref Cell.Symbol(y.str, nil);
	* =>
		cell->error("non-string argument to string->symbol\n");
	}
	return nil;
}

symbolp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to symbol?\n");
		return nil;
	}
	pick y := x {
	Symbol =>
		return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

sym2str(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in symbol->string\n");
		return nil;
	}
	pick y := x {
	Symbol =>
		return ref Cell.String(y.sym);
	* =>
		cell->error("non-symbol argument to symbol->string\n");
	}
	return nil;
}

tan(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in tan\n");
		return nil;
	}
	pick y := x {
	Number =>
		n := math->tan(y.r);
		return ref Cell.Number(big n, big 1, n, 0);
	* =>
		cell->error("non-numeric argument to tan\n");
	}
	return nil;
}

truncate(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to truncate\n");
		return nil;
	}
	pick y := x {
	Number =>
		math->FPcontrol(math->RND_Z, math->RND_MASK);
		n := math->rint(y.r);
		math->FPcontrol(math->RND_NR, math->RND_MASK);
		return ref Cell.Number(big n, big 1, n, cell->Exact);
	* =>
		cell->error("non-numeric argument to truncate\n");
	}
	return nil;
}

values(nil: ref Cell): ref Cell
{
	return ref Cell.Link(nil);
}

vectorp(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in vector?\n");
		return nil;
	}
	pick y := x {
	Vector =>
		return ref Cell.Boolean(1);
	}
	return ref Cell.Boolean(0);
}

vectorlen(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments in vector-length\n");
		return nil;
	}
	pick y := x {
	Vector =>
		z := len y.v;
		return ref Cell.Number(
			big z, big 1, real z, cell->Integer|cell->Exact);
	* =>
		cell->error("non-vector argument to vector-length\n");
	}
	return nil;
}

vectorref(args: ref Cell): ref Cell
{
	k: int;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	if(x == nil || y == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in vector-ref\n");
		return nil;
	}
	pick y1 := y {
	Number =>
		k = int y1.i;
	* =>
		cell->error("non-numeric index in vector-ref\n");
		return nil;
	}
	pick x1 := x {
	Vector =>
		return x1.v[k];
	* =>
		cell->error("non-vector argument to vector-ref\n");
	}
	return nil;
}

vectorset(args: ref Cell): ref Cell
{
	k: int;

	x := cell->lcar(args);
	l := cell->lcdr(args);
	y := cell->lcar(l);
	l = cell->lcdr(l);
	z := cell->lcar(l);
	if(x == nil || y == nil || z == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments to vector-set!\n");
		return nil;
	}
	pick y1 := y {
	Number =>
		k = int y1.i;
	* =>
		cell->error("non-numeric index in vector-set!\n");
		return nil;
	}
	pick x1 := x {
	Vector =>
		x1.v[k] = z;
	* =>
		cell->error("non-vector argument to vector-set!\n");
	}
	return z;
}

lwrite(args: ref Cell): ref Cell
{
	port := stdout;
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(l != nil && !(cell->isnil(l))) {
		p := cell->lcar(l);
		if(p == nil) {
			cell->error("invalid port in write\n");
			return nil;
		}
		pick q := p {
		Port =>
			if(q.dir != -1)
				port = q.p;
			else {
				cell->error("non-open port in write\n");
				return nil;
			}
		}
	}
	printcell(x, port, 0);
	return x;
}

writechar(args: ref Cell): ref Cell
{
	port := stdout;
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to write-char\n");
		return nil;
	}
	l := cell->lcdr(args);
	if(l != nil && !(cell->isnil(l))) {
		p := cell->lcar(l);
		if(p == nil) {
			cell->error("invalid port in write-char\n");
			return nil;
		}
		pick q := p {
		Port =>
			if(q.dir != -1)
				port = q.p;
			else {
				cell->error("non-open port in write-char\n");
				return nil;
			}
		}
	}
	pick y := x {
	Char =>
		port.putc(y.c);
	* =>
		cell->error("non-character argument to write-char\n");
		return nil;
	}
	port.flush();
	return x;
}

binstr(n: big): string
{
	if(n == big 0)
		return "0";
	s := "";
	while(n != big 0) {
		if((n & big 1) == big 1)
			s = "1" + s;
		else
			s = "0" + s;
		n >>= 1;
	}
	if(len s != 64)
		s = "0" + s;
	return s;
}

