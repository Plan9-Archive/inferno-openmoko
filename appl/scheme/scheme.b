#
# TODO:
# Sec 6.4 control features
# proper tail recursion
# define-syntax
# alt and/or spawn
#
implement Scheme;

include "sys.m";
sys: Sys;

include "bufio.m";
bufio: Bufio;
Iobuf: import bufio;

include "string.m";
str: String;

include "math.m";
math: Math;

include "draw.m";

include "sh.m";

include "cell.m";
cell: SCell;
Cell: import cell;
Pair: import cell;
Env: import cell;

include "sform.m";
sform: SForm;

include "builtin.m";
builtin: BuiltIn;

include "scheme.m";

stdin, stdout: ref Iobuf = nil;

ctxt: ref Draw->Context;

init(drawctxt: ref Draw->Context, nil: list of string)
{
	sys = load Sys Sys->PATH;
	bufio = load Bufio Bufio->PATH;
	str = load String String->PATH;
	math = load Math Math->PATH;
	cell = load SCell SCell->PATH;
	sform = load SForm SForm->PATH;
	builtin = load BuiltIn BuiltIn->PATH;

	ctxt = drawctxt;
	stdin = bufio->fopen(sys->fildes(0), Bufio->OREAD);
	stdout = bufio->fopen(sys->fildes(1), Bufio->OWRITE);

	cell->init(sys);

	e := ref Env("nil", cell->Variable, ref Cell.Link(nil), nil) :: nil;
	e = ref Env("else", cell->Variable, ref Cell.Boolean(1), nil) :: nil;
	cell->envstack = e;

	sform->init(sys, load Scheme SELF, cell);

	cell->nullenvironment = cell->envstack;

	builtin->init(sys, load Scheme SELF, cell, math, str,
		 bufio, stdin, stdout);

	b := bufio->open("/lib/scheme/library.scm", Bufio->OREAD);
	if(b == nil) {
		cell->error("Can't open library code\n");
	}
	else {
		while(1) {
			c := readcell(b);
			if(c == nil)
				break;
			eval(c);
		}
	}
	b = nil;
	cell->reportenv = cell->envstack;
	e = ref Env("popen", cell->BuiltIn, nil, popen) :: cell->globalenv;
	cell->globalenv = e;
	x := hd cell->globalenv;
	x.val = ref Cell.Symbol(x.name, x);

	while(1) {
		sys->print("> ");
		c := readcell(stdin);
		r := eval(c);
		printcell(r, stdout, 0); stdout.flush(); sys->print("\n");
	}
}

popen(args: ref Cell): ref Cell
{
	cmd: string;
	r: ref Cell;

	r = nil;
	x := cell->lcar(args);
	if(x == nil) {
		cell->error("wrong number of arguments to popen\n");
		return nil;
	}
	pick name := x {
	String =>
		cmd = name.str;
	* =>
		cell->error("non-string argument to popen\n");
		return nil;
	}
	infds := array[2] of ref Sys->FD;
	outfds := array[2] of ref Sys->FD;
	sys->pipe(infds);
	sys->pipe(outfds);
	spawn startshell(cmd, outfds[0], infds[1]);
	outfds[0] = nil;
	infds[1] = nil;
	rb := bufio->fopen(infds[0], Bufio->OREAD);
	tb := bufio->fopen(outfds[1], Bufio->OWRITE);
	rc := ref Cell.Port(rb, Bufio->OREAD);
	tc := ref Cell.Port(tb, Bufio->OWRITE);
	return cell->lcons(rc, cell->lcons(tc, ref Cell.Link(nil)));
}

startshell(cmd: string, infd: ref Sys->FD, outfd: ref Sys->FD)
{
	sh := load Sh Sh->PATH;
	if(sh == nil) {
		sys->print("loading sh failed: %r\n");
		exit;
	}
	sys->pctl(Sys->NEWFD, 2 :: infd.fd :: outfd.fd :: nil);
	sys->dup(infd.fd, 0);
	sys->dup(outfd.fd, 1);
	infd = nil;
	outfd = nil;
	sh->init(ctxt, "sh" :: "-c" :: cmd :: nil);
	cell->error(sys->sprint("child shell returned: %r\n"));
}

readcell(b: ref Iobuf): ref Cell
{
	c: int;

	while(1) {
		do {
			c = b.getc();
		} while(c != Bufio->EOF && str->in(c, " \t\n"));
		case c {
		Bufio->EOF or ')' =>
			return nil;
		';' =>
			do {
				c = b.getc();
			} while(c != Bufio->EOF && c != '\n');
		'(' =>
			return readlist(b);
		'"' =>
			return readstring(b);
		'\'' =>
			return cell->lcons(ref Cell.Symbol("quote",
				cell->lookupsym("quote")),
				cell->lcons(readcell(b), ref Cell.Link(nil)));
		'`' =>
			return cell->lcons(ref Cell.Symbol("quasiquote",
				cell->lookupsym("quasiquote")),
				cell->lcons(readcell(b), ref Cell.Link(nil)));
		',' =>
			c = b.getc();
			if(c == '@')
				return cell->lcons(ref Cell.Symbol("unquote-splicing",
					cell->lookupsym("unquote-splicing")),
					cell->lcons(readcell(b), ref Cell.Link(nil)));
			else {
				b.ungetc();
				return cell->lcons(ref Cell.Symbol("unquote",
					cell->lookupsym("unquote")),
					cell->lcons(readcell(b), ref Cell.Link(nil)));
			}
		'+' or '-' or '.' or '0' to '9' =>
			b.ungetc();
			return readnumber(b, 0);
		'#' =>
			c = b.getc();
			case c {
			'b' or 'B' or 'o' or 'O' or 'd' or 'D' or 'x' or 'X' 
			or 'e' or 'E' or 'i' or 'I' =>
				b.ungetc();
				return readnumber(b, '#');
			'f' =>
				return ref Cell.Boolean(0);
			't' =>
				return ref Cell.Boolean(1);
			'\\' =>
				return readchar(b);
			'(' =>
				return readvector(b);
			}
		* =>
			b.ungetc();
			return readsymbol(b);
		}
	}
	return nil;
}

readchar(b: ref Iobuf): ref Cell
{
	lexeme: string;

	lexeme[0] = b.getc();
	i := 1;
	do {
		lexeme[i] = b.getc();
	} while(!str->in(lexeme[i++], " \t\n\r();"));
	b.ungetc();
	lexeme = lexeme[:len lexeme -1];
	if(len lexeme == 1)
		return ref Cell.Char(lexeme[0]);
	case lexeme {
	"space" =>
		return ref Cell.Char(' ');
	"newline" =>
		return ref Cell.Char('\n');
	"return" =>
		return ref Cell.Char('\r');
	"tab" =>
		return ref Cell.Char('\t');
	"backspace" =>
		return ref Cell.Char('\b');
	"bell" or "alert" =>
		return ref Cell.Char('\a');
	"quote" =>
		return ref Cell.Char('\'');
	"doublequote" =>
		return ref Cell.Char('\"');
	"null" =>
		return ref Cell.Char(0);
	"backslash" =>
		return ref Cell.Char('\\');
	"escape" =>
		return ref Cell.Char(16r1b);
	"formfeed" =>
		return ref Cell.Char('\v');
	}
	return nil;
}

readvector(b: ref Iobuf): ref Cell
{
	l: list of ref Cell;

	l = nil;
	while(1) {
		c := readcell(b);
		if(c == nil)
			break;
		l = c :: l;
	}
	v := array [len l] of ref Cell;
	for(i := len l - 1; i >= 0; --i) {
		v[i] = hd l;
		l = tl l;
	}
	return ref Cell.Vector(v);
}

readlist(b: ref Iobuf): ref Cell
{
	c := readcell(b);
	if(c == nil)
		return ref Cell.Link(nil);
	pick x := c {
	Symbol =>
		if(x.sym == ".") {
			cdr := readcell(b);
			if(readcell(b) != nil) {
				cell->error("malformed improper list\n");
				return nil;
			}
			return cdr;
		}
	}
	p := Pair(nil, nil);
	p.car = c;
	p.cdr = readlist(b);
	return ref Cell.Link(ref p);
}

readstring(b: ref Iobuf): ref Cell
{
	x: string;

	i := 0;
	esc := 0;
loop:
	while(1) {
		x[i] = b.getc();
		case x[i] {
		'"' =>
			if(!esc)
				break loop;
			else {
				++i;
				esc = 0;
			}
		'\\' =>
			if(esc) {
				++i;
				esc = 0;
			}
			else
				esc = 1;
		't' =>
			if(esc) {
				x[i] = '\t';
				esc = 0;
			}
			++i;
		'n' =>
			if(esc) {
				x[i] = '\n';
				esc = 0;
			}
			++i;
		'r' =>
			if(esc) {
				x[i] = '\r';
				esc = 0;
			}
			++i;
		'b' =>
			if(esc) {
				x[i] = '\b';
				esc = 0;
			}
			++i;
		'a' =>
			if(esc) {
				x[i] = '\a';
				esc = 0;
			}
			++i;
		'v' =>
			if(esc) {
				x[i] = '\v';
				esc = 0;
			}
			++i;
		* =>
			esc = 0;
			++i;
		}

	}
	return ref Cell.String(x[:i]);
}

readnumber(b: ref Iobuf, seed: int): ref Cell
{
	s: string;

	# Get the string
	i := -1;
	if(seed != 0)
		s[++i] = seed;
	do {
		++i;
		s[i] = b.getc();
	} while(str->in(s[i], "-+.#oOxXiIsSlL0-9A-Fa-f/"));
	b.ungetc();
	if(s[:i] == ".")
		return ref Cell.Symbol(".", nil);
	else if(s[:i] == "+")
		return ref Cell.Symbol("+", cell->lookupsym("+"));
	else if(s[:i] == "-")
		return ref Cell.Symbol("-", cell->lookupsym("-"));
	--i;
	return scannum(s, 10);
}

readsymbol(b: ref Iobuf): ref Cell
{
	x: string;

	i := 0;
	do {
		x[i] = b.getc();
	} while(!str->in(x[i++], " \t\n()"));
	b.ungetc();
	e := cell->lookupsym(x[:i-1]);
	if(e != nil && e.ilk != cell->SpecialForm && e.ilk != cell->BuiltIn)
		e = nil;
	return ref Cell.Symbol(x[:i-1], e);
}

bugger: int;

eval(c: ref Cell): ref Cell
{
	if(c == nil || cell->isnil(c))
		return c;
	pick x := c {
	Link =>
		if(x.next == nil)
			return ref Cell.Link(nil);
		r := eval(x.next.car);
		if(r == nil) {
			cell->error("Undefined operation: ");
			printcell(x.next.car, stdout, 0);
			stdout.putc('\n');
			return nil;
		}
		pick y := r {
		Symbol =>
#			if(y.env == nil)
				e := cell->lookupsym(y.sym);
#			else
#				e = y.env;
			if(e == nil)
				return nil;
			case e.ilk {
			cell->BuiltIn =>
				l := evallist(x.next.cdr);
				return e.handler(l);
			cell->SpecialForm =>
				return e.handler(x.next.cdr);
			cell->Variable =>
				return eval(e.val);
			}
		Lambda =>
			saveenv := cell->envstack;
			l := evallist(x.next.cdr);
			cell->envstack = cell->listappend(y.env, cell->envstack);
			p := y.formals;
			q := l;
			dorest := 0;
			while(p != nil && q != nil) {
				fname := "";
				pick fp := p {
				Link =>
					if(fp.next != nil && fp.next.car != nil) {
						pick ffp := fp.next.car {
						Symbol =>
							fname = ffp.sym;
						* =>
							cell->error("non-symbol in formals\n");
							cell->envstack = saveenv;
							return nil;
						}
						p = fp.next.cdr;
					}
					else
						p = nil;
				Symbol =>
					fname = fp.sym;
					dorest = 1;
					p = nil;
				* =>
					p = nil;
				}
				pick vp := q {
				Link =>
					if(vp.next != nil) {
						if(dorest) {
							(nil, el) := cell->ldefine(
								fname, vp, cell->envstack);
							cell->envstack = el;
							q = nil;
						}
						else {
							(nil, el) := cell->ldefine(fname,
								vp.next.car, cell->envstack);
							cell->envstack = el;
							q = vp.next.cdr;
						}
					}
					else {
						if(dorest) {
							(nil, el) := cell->ldefine(fname,
								 ref Cell.Link(nil), cell->envstack);
							cell->envstack = el;
						}
						q = nil;
					}
				* =>
					q = nil;
				}
			}
			if(p != nil || q != nil) {
				cell->error("wrong number of arguments\n");
				cell->envstack = saveenv;
				return nil;
			}
			exp := y.exp_list;
			r: ref Cell;
			r = ref Cell.Link(nil);
			while(exp != nil) {
				pick ep := exp {
				Link =>
					if(ep.next != nil) {
						r = eval(ep.next.car);
						if(r == nil) {
							cell->envstack = saveenv;
							return nil;
						}
						exp = ep.next.cdr;
					}
					else
						exp = nil;
				* =>
					cell->error("malformed expression list\n");
					cell->envstack = saveenv;
					return nil;
				}
			}
			cell->envstack = saveenv;
			return r;
		* =>
			cell->error("non-lambda and non-symbol in eval\n");
			return nil;
		}
	Symbol =>
#		if(x.env == nil)
			s := cell->lookupsym(x.sym);
#		else
#			s = x.env;
		if(s == nil)
			return nil;
		else
			return s.val;
	* =>
		return c;
	}
	return nil;
}

evallist(c: ref Cell): ref Cell
{
	if(c == nil || cell->isnil(c))
		return c;
	pick x := c {
	Link =>
		if(x.next == nil)
			return ref Cell.Link(nil);
		vc := eval(x.next.car);
		if(vc == nil)
			return nil;
		vl := evallist(x.next.cdr);
		if(vl == nil)
			return nil;
		y := Pair(vc, vl);
		return ref Cell.Link(ref y);
	* =>
		cell->error("non-list in evallist\n");
	}
	return nil;
}

printlist(plist: ref Pair, b: ref Iobuf, disp: int)
{
	x: ref Pair;

	b.puts("(");
	x = plist;
	while(x != nil) {
		printcell(x.car, b, disp);
		y := x.cdr;
		if(y == nil) {
			cell->error("unexpected end of list\n");
			break;
		}
		pick z := y {
		Link =>
			x = z.next;
			if(x != nil)
				b.puts(" ");
		* =>
			b.puts(" . ");
			printcell(z, b, disp);
			x = nil;
		}
	}
	b.puts(")");
	b.flush();
}

printvector(v: array of ref Cell, b: ref Iobuf, disp: int)
{
	b.puts("#(");
	i := 0;
	while(1) {
		printcell(v[i], b, disp);
		if(++i >= len v)
			break;
		b.putc(' ');
	}
	b.puts(")");
	b.flush();
}

printcell(x: ref Cell, b: ref Iobuf, disp: int)
{
	if(x == nil) {
		b.puts("nil");
		b.flush();
		return;
	}
	pick y := x {
	Boolean =>
		if(y.b == 0)
			b.puts("#f");
		else
			b.puts("#t");
	Symbol =>
		b.puts(sys->sprint("%s", y.sym));
	String =>
		if(disp)
			b.puts(y.str);
		else {
			b.putc('"');
			for(i := 0; i < len y.str; ++i)
				if(y.str[i] == '"')
					b.puts("\\\"");
				else
					b.putc(y.str[i]);
			b.putc('"');
		}
	Char =>
		if(disp)
			b.putc(y.c);
		else
		case y.c {
		'\n' =>
			b.puts("#\\newline");
		'\t' =>
			b.puts("#\\tab");
		'\r' =>
			b.puts("#\\return");
		'\b' =>
			b.puts("#\\backspace");
		'\a' =>
			b.puts("#\\bell");
		'\v' =>
			b.puts("#\\formfeed");
		* =>
			b.puts(sys->sprint("#\\%c", y.c));
		}
	Number =>
		if(y.ilk & cell->Exact) {
			if(y.j == big 1)
				b.puts(sys->sprint("%bd", y.i));
			else
				b.puts(sys->sprint("%bd/%bd", y.i, y.j));
		}
		else
			b.puts(sys->sprint("%.#g", y.r));
	Link =>
		printlist(y.next, b, disp);
	Lambda =>
		b.puts("[lambda expression]");
	Port =>
		case y.dir {
		-1 =>
			b.puts("[closed port]");
		Bufio->OREAD =>
			b.puts("[input port]");
		Bufio->OWRITE =>
			b.puts("[output port]");
		}
	Vector =>
		printvector(y.v, b, disp);
	Environment =>
		b.puts("[environment]");
	}
	b.flush();
}

scannum(s: string, radix: int): ref Cell
{
	n2: big;

	exact := cell->Exact;
	ilk := 0;
	sign := big 1;

	# parse the prefix
	j := 0;
	for(k := 0; k < 2; ++k) {
		if(s[j] != '#')
			break;
		++j;
		case s[j] {
		'b' or 'B' =>
			radix = 2;
		'o' or 'O' =>
			radix = 8;
		'd' or 'D' =>
			radix = 10;
		'x' or 'X' =>
			radix = 16;
		'e' or 'E' =>
			exact = cell->Exact;
		'i' or 'I' =>
			exact = 0;
		}
		++j;
	}

	# Get the initial sign
	if(s[j] == '+') {
		sign = big 1;
	}
	else if(s[j] == '-') {
		sign = big -1;
	}

	# Try to classify the number (ugly ad hoc)
	# As a first cut, just integers and reals
	(s1,s2) := str->splitl(s[j:], ".eEsSfFdDlL");
	if(s2 == nil) {
		(s1, s2) = str->splitl(s[j:], "/");
		(n1, nil) := str->tobig(s1, radix);
		if(s2 != nil) {
			ilk = cell->Rational;
			(n2, nil) = str->tobig(s2[1:], radix);
		}
		else {
			ilk = cell->Integer;
			n2 = big 1;
		}
		if(n2 != big 1)
			(n1, n2) = reduce(n1, n2);
		return ref Cell.Number(n1, n2, real n1 / real n2, ilk|exact);
	}
	else {
		for(m := j; m < len s; ++m) {
			case s[m] {
			's' or 'S' or 'f' or 'F' or 'd' or 'D' or 'l' or 'L' =>
				s[m] = 'e';
			}
		}
		n := real s[j:];
		return ref Cell.Number(big n, big 1, n, cell->Real);
	}
}

printenv()
{
	sys->print("\n***Env: ");
	for(p := cell->envstack; p != nil; p = tl p) {
		sys->print("%s:", (hd p).name);
		printcell((hd p).val, stdout, 0);
		sys->print(" ");
	}
	sys->print("\n");
}

# Basically Euclid's gcd algorithm
reduce(n, m: big): (big,big)
{
	j := n;
	k := m;
	while(1) {
		r := j % k;
		if(r == big 0)
			return (n/k, m/k);
		j = k;
		k = r;
	}
}

