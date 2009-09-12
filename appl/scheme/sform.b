implement SForm;

include "sys.m";
include "draw.m";

include "bufio.m";
bufio: Bufio;
Iobuf: import bufio;

include "cell.m";
cell: SCell;
Cell: import cell;
Pair: import cell;
Env: import cell;

include "scheme.m";
scheme: Scheme;
eval: import scheme;

include "sform.m";

init(nil: Sys, sch: Scheme, c: SCell)
{
	cell = c;
	scheme = sch;

	e := cell->envstack;
	e = ref Env("quote", cell->SpecialForm, nil, quote) :: e;
	e = ref Env("quasiquote", cell->SpecialForm, nil, qquote) :: e;
	e = ref Env("define", cell->SpecialForm, nil, define) :: e;
	e = ref Env("if", cell->SpecialForm, nil, ifsf) :: e;
	e = ref Env("lambda", cell->SpecialForm, nil, lambda) :: e;
	e = ref Env("set!", cell->SpecialForm, nil, setbang) :: e;
	e = ref Env("unquote", cell->SpecialForm, nil, unquote) :: e;
	e = ref Env("unquote-splicing", cell->SpecialForm, nil, unquotesplice) :: e;
	e = ref Env("and", cell->SpecialForm, nil, land) :: e;
	e = ref Env("begin", cell->SpecialForm, nil, begin) :: e;
	e = ref Env("or", cell->SpecialForm, nil, lor) :: e;
	e = ref Env("case", cell->SpecialForm, nil, lcase) :: e;
	e = ref Env("cond", cell->SpecialForm, nil, cond) :: e;
	e = ref Env("let", cell->SpecialForm, nil, let) :: e;
	e = ref Env("let*", cell->SpecialForm, nil, letstar) :: e;
	e = ref Env("letrec", cell->SpecialForm, nil, letrec) :: e;
	cell->envstack = e;
	l := e;
	while(l != nil) {
		x := hd l;
		if(x.ilk == cell->BuiltIn || x.ilk == cell->SpecialForm)
			x.val = ref Cell.Symbol(x.name, x);
		l = tl l;
	}
}

land(args: ref Cell): ref Cell
{
	c: ref Cell;

	c = ref Cell.Boolean(1);
	p := cell->lcar(args);
	l := cell->lcdr(args);
	while(p != nil && !(cell->isnil(p))) {
		c = eval(p);
		pick cn := c {
		Boolean =>
			if(cn.b == 0)
				return c;
		}
		if(l == nil || cell->isnil(l))
			break;
		p = cell->lcar(l);
		l = cell->lcdr(l);
	}
	return c;
}

begin(args: ref Cell): ref Cell
{
	c: ref Cell;

	p := cell->lcar(args);
	if(p == nil) {
		cell->error("wrong number of arguments in begin\n");
		return nil;
	}
	l := cell->lcdr(args);
	while(p != nil && !(cell->isnil(p))) {
		c = eval(p);
		if(l == nil || cell->isnil(l))
			break;
		p = cell->lcar(l);
		l = cell->lcdr(l);
	}
	return c;
}

lcase(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil || cell->isnil(l)) {
		cell->error("wrong number of expressions in case\n");
		return nil;
	}
	key := eval(cell->lcar(args));
	if(key == nil) {
		cell->error("key expression missing in case\n");
		return nil;
	}
	do {
		clause := cell->lcar(l);
		if(clause == nil || cell->isnil(clause)) {
			cell->error("non-pair clause in case\n");
			return nil;
		}
		data := cell->lcar(clause);
		if(data == nil || cell->isnil(data)) {
			cell->error("non-pair clause in case\n");
			return nil;
		}
		exprs := cell->lcdr(clause);
		if(exprs == nil || cell->isnil(exprs)) {
			cell->error("non-pair clause in case\n");
			return nil;
		}
		pick elp := data {
		Symbol =>
			if(elp.sym == "else")
				return begin(exprs);
		}
		dl := data;
		do {
			datum := cell->lcar(dl);
			if(cell->leqvp(key, datum) == 1)
				return begin(exprs);
			dl = cell->lcdr(dl);
		} while(dl != nil && !(cell->isnil(dl)));
		l = cell->lcdr(l);
	} while(l != nil && !(cell->isnil(l)));
	return nil;
}

procel(res, el: ref Cell): ref Cell
{
	if(el == nil || cell->isnil(el))
		return res;
	pick arrow := cell->lcar(el) {
	Symbol =>
		if(arrow.sym == "=>") {
			l := cell->lcdr(el);
			if(l == nil || cell->isnil(l))
				return nil;
			c := eval(cell->lcar(l));
			qr := cell->lcons(ref Cell.Symbol("quote", nil),
				cell->lcons(res, ref Cell.Link(nil)));
			return eval(cell->lcons(c, cell->lcons(qr, ref Cell.Link(nil))));
		}
	}
	return begin(el);
}

cond(args: ref Cell): ref Cell
{
	cl := cell->lcar(args);
	l := cell->lcdr(args);
	if(cl == nil || cell->isnil(cl) || l == nil || cell->isnil(l)) {
		cell->error("wrong number of arguments in cond\n");
		return nil;
	}
	while(1) {
		test := cell->lcar(cl);
		if(test == nil || cell->isnil(test)) {
			cell->error("invalid test in cond\n");
			return nil;
		}
		res := eval(test);
		el := cell->lcdr(cl);
		pick r := res {
		Boolean =>
			if(r.b == 1)
				return procel(res, el);
		* =>
			return procel(res, el);
		}
		if(l == nil || cell->isnil(l))
			break;
		cl = cell->lcar(l);
		l = cell->lcdr(l);
	}
	return nil;
}

define(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	l := cell->lcdr(args);
	if(x == nil || l == nil) {
		cell->error("wrong number of arguments in define\n");
		return nil;
	}
	pick y := x {
	Symbol =>
		e := cell->lookupsym(y.sym);
		if(e != nil) {
			e.val = eval(cell->lcar(l));
			return ref Cell.Symbol(y.sym, e);
		}
		(c, el) := cell->ldefine(y.sym, eval(cell->lcar(l)), cell->globalenv);
		cell->globalenv = el;
		return c;
	Link =>
		pick z := cell->lcar(x) {
		Symbol =>
			lc := ref Cell.Symbol("lambda", cell->lookupsym("lambda"));
			fp := ref Cell.Link(ref Pair(cell->lcdr(x), l));
			lp := ref Cell.Link(ref Pair(lc, fp));
			e := cell->lookupsym(z.sym);
			if(e != nil) {
				e.val = eval(lp);
				return ref Cell.Symbol(z.sym, e);
			}
			(c, el) := cell->ldefine(z.sym, eval(lp), cell->globalenv);
			cell->globalenv = el;
			return c;
		}
	}
	return ref Cell.Link(nil);
}

ifsf(args: ref Cell): ref Cell
{
	e3: ref Cell;

	e1 := cell->lcar(args);
	l := cell->lcdr(args);
	e2 := cell->lcar(l);
	if(e1 == nil || e2 == nil || l == nil) {
		cell->error("wrong number of expressions in if\n");
		return nil;
	}
	l = cell->lcdr(l);
	if(l == nil || cell->isnil(l))
		e3 = ref Cell.Link(nil);
	else
		e3 = cell->lcar(l);
	truth := eval(e1);
	pick x := truth {
	Boolean =>
		if(x.b == 0)
			return eval(e3);
	}
	return eval(e2);
}

lambda(args: ref Cell): ref Cell
{
	if(args == nil) {
		cell->error("too few arguments in lambda expressions\n");
		return nil;
	}
	pick x := args {
	Link =>
		if(x.next == nil || x.next.cdr == nil)
			return ref Cell.Link(nil);
		return ref Cell.Lambda(x.next.car,
			x.next.cdr, cell->envstack);
	}
	cell->error("invalid lambda expression\n");
	return nil;	
}

let(args: ref Cell): ref Cell
{
	vals: list of (string, ref Cell);

	if(args == nil || cell->isnil(args)) {
		cell->error("too few arguments in let\n");
		return nil;
	}
	binds := cell->lcar(args);
	exprs := cell->lcdr(args);
	if(binds == nil || cell->isnil(binds))
		return begin(exprs);
	func_name := "";
	pick x := binds {
	Symbol =>
		func_name = x.sym;
		binds = cell->lcar(exprs);
		exprs = cell->lcdr(exprs);
	}
	vals = nil;
	bl := binds;
	do {
		b := cell->lcar(bl);
		if(b == nil || cell->isnil(b))
			break;
		exp := cell->lcdr(b);
		pick var := cell->lcar(b) {
		Symbol =>
			vals = (var.sym, begin(exp)) :: vals;
		}
		bl = cell->lcdr(bl);
	} while(bl != nil && !(cell->isnil(bl)));
	saveenv := cell->envstack;
	bl = binds;
	do {
		b := cell->lcar(bl);
		if(b == nil || cell->isnil(b))
			break;
		if(vals == nil)
			break;
		(var, val) := hd vals;
		(nil, el) := cell->ldefine(var, val, cell->envstack);
		cell->envstack = el;
		bl = cell->lcdr(bl);
		vals = tl vals;
	} while(bl != nil && !(cell->isnil(bl)));
	if(func_name != "") {
		bl = binds;
		formals := ref Cell.Link(nil);
		f: ref Cell;
		f = formals;
		do {
			fname: string;
			b := cell->lcar(bl);
			if(b == nil || cell->isnil(b))
				break;
			pick bn := cell->lcar(b) {
			Symbol =>
				fname = bn.sym;
			}
			pick fl := f {
			Link =>
				fl.next = ref Pair(
					ref Cell.Symbol(fname, nil), ref Cell.Link(nil));
				f = cell->lcdr(f);
			}
			bl = cell->lcdr(bl);
		} while(bl != nil && !(cell->isnil(bl)));
		lambda_exp := cell->lcons(
			ref Cell.Symbol("lambda", cell->lookupsym("lambda")),
				cell->lcons(formals,
				cell->lcons(cell->lcar(exprs), ref Cell.Link(nil))));
		(nil, el) := cell->ldefine(
			func_name, eval(lambda_exp), cell->envstack);
		cell->envstack = el;
	}
	res := begin(exprs);
	cell->envstack = saveenv;
	return res;
}

letstar(args: ref Cell): ref Cell
{
	if(args == nil || cell->isnil(args)) {
		cell->error("too few arguments to let*\n");
		return nil;
	}
	binds := cell->lcar(args);
	exprs := cell->lcdr(args);
	if(binds == nil || cell->isnil(binds))
		return begin(exprs);
	saveenv := cell->envstack;
	bl := binds;
	do {
		b := cell->lcar(bl);
		if(b == nil || cell->isnil(b))
			break;
		pick var := cell->lcar(b) {
		Symbol =>
			exp := cell->lcdr(b);
			(nil, el) := cell->ldefine(
				var.sym, begin(exp), cell->envstack);
			cell->envstack = el;
		}
		bl = cell->lcdr(bl);
	} while(bl != nil && !(cell->isnil(bl)));
	res := begin(exprs);
	cell->envstack = saveenv;
	return res;
}

letrec(args: ref Cell): ref Cell
{
	return letstar(args);
}

lor(args: ref Cell): ref Cell
{
	c: ref Cell;

	if(args == nil)
		return nil;
	if(cell->isnil(args))
		return ref Cell.Boolean(0);
	p := cell->lcar(args);
	l := cell->lcdr(args);
	while(p != nil && !(cell->isnil(p))) {
		c = eval(p);
		pick cn := c {
		Boolean =>
			if(cn.b == 1)
				return c;
		* =>
			return c;
		}
		if(l == nil || cell->isnil(l))
			break;
		p = cell->lcar(l);
		l = cell->lcdr(l);
	}
	return c;
}

lqquote(expr: ref Cell, level: int): (int, ref Cell)
{
	if(expr == nil || cell->isnil(expr))
		return (0, expr);
	pick y := expr {
	Link =>
		if(y.next == nil || y.next.car == nil)
			return (0, expr);
		pick z := y.next.car {
		Symbol =>
			if(z.sym == "unquote") {
				if(level == 1)
					return (0, unquote(y.next.cdr));
				else {
					(nil, c) := lqquote(y.next.cdr, level - 1);
					return (0, ref Cell.Link(ref Pair(z, c)));
				}
			}
			if(z.sym == "unquote-splicing") {
				if(level == 1)
					return (1, unquote(y.next.cdr));
				else {
					(nil, c) := lqquote(y.next.cdr, level - 1);
					return (0, ref Cell.Link(ref Pair(z, c)));
				}
			}
			if(z.sym == "quasiquote") {
				(nil, c) := lqquote(y.next.cdr, level + 1);
				return (0, ref Cell.Link(ref Pair(z, c)));
			}
		}
		(n, ca) := lqquote(y.next.car, level);
		(nil, cd) := lqquote(y.next.cdr, level);
		if(n == 1)
			return (0, cell->lappend(ca, cd));
		else
			return (0, ref Cell.Link(ref Pair(ca, cd)));
	Vector =>
		n := len y.v;
		nl: list of ref Cell;
		nl = nil;
		for(i := 0; i < n; ++i) {
			(qqs, c) := lqquote(y.v[i], level);
			if(qqs == 0) {
				nl = c :: nl;
			}
			else {
				p := c;
				while(1) {
					if(p == nil || cell->isnil(p))
						break;
					nl = cell->lcar(p) :: nl;
					p = cell->lcdr(p);
				}
			}
		}
		nv := array[len nl] of ref Cell;
		for(i = len nl - 1; i >= 0; --i) {
			nv[i] = hd nl;
			nl = tl nl;
		}
		return (0, ref Cell.Vector(nv));
	* =>
		return (0, expr);
	}
}

qquote(args: ref Cell): ref Cell
{
	if(args == nil || cell->isnil(args)) {
		cell->error("wrong number of arguments to quasiquote\n");
		return nil;
	}
	(nil, c) := lqquote(cell->lcar(args), 1);
	return c;
}

quote(args: ref Cell): ref Cell
{
	if(args == nil || cell->isnil(args))
		return nil;
	return cell->lcar(args);
}

setbang(args: ref Cell): ref Cell
{
	if(args == nil || cell->isnil(args))
		return nil;
	p := cell->lcar(args);
	if(p == nil || cell->isnil(p))
		return nil;
	l := cell->lcdr(args);
	if(l == nil || cell->isnil(l))
		return nil;
	pick y := p {
	Symbol =>
		e := cell->lookupsym(y.sym);
		if(e == nil) {
			cell->error("Cannot set unbound variable\n");
			return nil;
		}
		e.val = eval(cell->lcar(l));
	}
	return p;
}

unquote(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to unquote\n");
		return nil;
	}
	return eval(x);
}

unquotesplice(args: ref Cell): ref Cell
{
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to unquote-splicing\n");
		return nil;
	}
	c := eval(x);
	if(c == nil || cell->isnil(c)) {
		cell->error("invalid expression in unquote-splicing\n");
		return nil;
	}
	pick y := c {
	Link =>
		return y.next.car;
	* =>
		cell->error("invalid expression in unquote-splicing\n");
	}
	return nil;
}

