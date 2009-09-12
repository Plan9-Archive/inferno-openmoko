SCell: module
{
	PATH: con "/dis/scheme/cell.dis";

	Cell: adt
	{
		pick {
		Boolean =>
			b: int;
		Symbol =>
			sym: string;
			env: ref Env;
		String =>
			str: string;
		Char =>
			c: int;
		Number =>
			i, j: big;
			r: real;
			ilk: int;
		Link =>
			next: cyclic ref Pair;
		Lambda =>
			formals: cyclic ref Cell;
			exp_list: cyclic ref Cell;
			env: list of ref Env;
		Vector =>
			v: cyclic array of ref Cell;
		Port =>
			p: ref Iobuf;
			dir: int;
		Environment =>
			env: list of ref Env;
		
		}
	};

	Integer,
	Rational,
	Real,
	Complex: con iota;
	Exact: con 16r80;

	Pair: adt
	{
		car: cyclic ref Cell;
		cdr: cyclic ref Cell;
	};

	Env: adt
	{
		name: string;
		ilk: int;
		val: cyclic ref Cell;
		handler: ref fn (args: ref Cell): ref Cell;
	};

	SpecialForm,
	BuiltIn,
	Procedure,
	Variable: con iota;

	baseenv: list of ref Env;
	reportenv, nullenvironment: list of ref Env;
	envstack: list of ref Env;
	globalenv: list of ref Env;

	init: fn(s: Sys);
	lcar: fn(args: ref Cell): ref Cell;
	lcdr: fn(args: ref Cell): ref Cell;
	lcons: fn(car: ref Cell, cdr: ref Cell): ref Cell;
	ldefine: fn(sym: string, exp: ref Cell, envlist: list of ref Env):
		(ref Cell, list of ref Env);
	leqp: fn(x1, x2: ref Cell): int;
	leqvp: fn(x1, x2: ref Cell): int;
	lappend: fn(c1, c2: ref Cell): ref Cell;
	isnil: fn(l: ref Cell): int;
	lookupsym: fn(symbl: string): ref Env;
	listappend: fn(l1, l2: list of ref Env): list of ref Env;
	error: fn(s: string);
};
