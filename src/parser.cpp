// Copyright (C) 2016 Mars Saxman. All rights reserved.
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided all source code distributions retain
// this paragraph and the above copyright notice. THIS SOFTWARE IS PROVIDED "AS
// IS" WITH NO EXPRESS OR IMPLIED WARRANTY.

#include "parser.h"

// Operators are grouped into precedence levels such that shifting the code
// right by 4 yields the precedence group number. Even-numbered groups have
// left associativity and odd-numbered groups have right associativity.


namespace op {
enum {
	// groups
	paren = 0x00,
	bracket,
	brace,

	// statements
	sequence = 0x10,

	// symbols
	capture = 0x30,
	define,

	// structure
	list = 0x50,
	caption,

	// relation
	equal = 0x60,
	lesser,
	greater,
	not_equal,
	not_lesser,
	not_greater,

	// additive
	add = 0x80,
	subtract,
	disjoin,
	exclude,
	range,

	// multiplicative
	multiply = 0xA0,
	divide,
	modulo,
	shift_left,
	shift_right,
	conjoin,

	// unary
	negate = 0xB0,
	complement,

	// primary
	lookup = 0xC0,
	subscript,
};
}

static int precedence(int op) {
	return op >> 4;
}

static bool rightassoc(int op) {
	return precedence(op) & 1;
}

void parser::token_number(location l, std::string text) {
	accept(out.rule_number(text));
}

void parser::token_symbol(location l, std::string text) {
	accept(out.rule_symbol(text));
}

void parser::token_string(location l, std::string text) {
	accept(out.rule_string(text));
}

void parser::token_underscore(location l) {
	accept(out.rule_blank());
}

void parser::token_paren_pair(location l) {
	if (!prefix) {
		states.push({op::subscript, l});
	}
	accept(out.rule_paren_empty());
}

void parser::token_paren(location l, direction d) {
	group(op::paren, &output::rule_paren_group, l, d);
}

void parser::token_bracket_pair(location l) {
	if (!prefix) {
		states.push({op::subscript, l});
	}
	accept(out.rule_bracket_empty());
}

void parser::token_bracket(location l, direction d) {
	group(op::bracket, &output::rule_bracket_group, l, d);
}

void parser::token_brace_pair(location l) {
	if (!prefix) {
		states.push({op::subscript, l});
	}
	accept(out.rule_brace_empty());
}

void parser::token_brace(location l, direction d) {
	group(op::brace, &output::rule_brace_group, l, d);
}

void parser::token_comma(location l) {
	parse_infix(op::list, l);
}

void parser::token_colon(location l) {
	parse_infix(op::caption, l);
}

void parser::token_semicolon(location l) {
	parse_infix(op::sequence, l);
}

void parser::token_dot(location l) {
	parse_infix(op::lookup, l);
}

void parser::token_dot_dot(location l) {
	parse_infix(op::range, l);
}

void parser::token_arrow(location l, direction d) {
	parse_directional(op::define, op::capture, l, d);
}

void parser::token_plus(location l) {
	parse_infix(op::add, l);
}

void parser::token_hyphen(location l) {
	if (prefix) {
		parse_prefix(op::negate, l);
	} else {
		parse_infix(op::subtract, l);
	}
}

void parser::token_star(location l) {
	parse_infix(op::multiply, l);
}

void parser::token_slash(location l) {
	parse_infix(op::divide, l);
}

void parser::token_percent(location l) {
	parse_infix(op::modulo, l);
}

void parser::token_equal(location l) {
	parse_infix(op::equal, l);
}

void parser::token_angle(location l, direction d) {
	parse_directional(op::lesser, op::greater, l, d);
}

void parser::token_bang_equal(location l) {
	parse_infix(op::not_equal, l);
}

void parser::token_bang_angle(location l, direction d) {
	parse_directional(op::not_lesser, op::not_greater, l, d);
}

void parser::token_bang(location l) {
	parse_prefix(op::complement, l);
}

void parser::token_ampersand(location l) {
	parse_infix(op::conjoin, l);
}

void parser::token_pipe(location l) {
	parse_infix(op::disjoin, l);
}

void parser::token_caret(location l) {
	parse_infix(op::exclude, l);
}

void parser::token_guillemet(location l, lexer::direction d) {
	parse_directional(op::shift_left, op::shift_right, l, d);
}

void parser::flush() {
	while (!states.empty()) {
		commit_op();
	}
}

void parser::accept(int val) {
	values.push(val);
	prefix = false;
}

int parser::recall() {
	int val = values.top();
	values.pop();
	return val;
}

void parser::group(int tk, int (output::*rule)(int), location l, direction d) {
	switch (d) {
		case direction::left: open_group(tk, l); break;
		case direction::right: close_group(tk, rule, l); break;
	}
}

void parser::open_group(int tk, location l) {
	if (!prefix) {
		states.push({op::subscript, l});
	}
	states.push({tk, l});
	prefix = true;
}

void parser::close_group(int tk, int (output::*rule)(int), location l) {
	prefix = false;
	while (!states.empty()) {
		if (states.top().id == tk) {
			states.pop();
			accept((out.*rule)(recall()));
			return;
		}
		commit_op();
	}
	err.parser_mismatched_group(l);
}

void parser::parse_directional(int l, int r, location loc, direction dir) {
	switch (dir) {
		case direction::left: parse_infix(l, loc); break;
		case direction::right: parse_infix(r, loc); break;
	}
}

void parser::parse_prefix(int tk, location l) {
	states.push({tk, l});
}

void parser::parse_infix(int tk, location l) {
	if (prefix) {
		err.parser_missing_operand(l);
		return;
	}
	while (!states.empty()) {
		int prev = states.top().id;
		if (precedence(tk) > precedence(prev)) break;
		if (rightassoc(tk) && precedence(tk) == precedence(prev)) break;
		commit_op();
	}
	states.push({tk, l});
	prefix = true;
}

void parser::commit_op() {
	token tk = states.top();
	states.pop();
	int v = recall();
	switch (tk.id) {
		case op::sequence: accept(out.rule_sequence(recall(), v)); break;
		case op::capture: accept(out.rule_capture(recall(), v)); break;
		case op::define: accept(out.rule_define(recall(), v)); break;
		case op::list: accept(out.rule_list(recall(), v)); break;
		case op::caption: accept(out.rule_caption(recall(), v)); break;
		case op::equal: accept(out.rule_equal(recall(), v)); break;
		case op::lesser: accept(out.rule_lesser(recall(), v)); break;
		case op::greater: accept(out.rule_greater(recall(), v)); break;
		case op::not_equal: accept(out.rule_not_equal(recall(), v)); break;
		case op::not_lesser: accept(out.rule_not_lesser(recall(), v)); break;
		case op::not_greater: accept(out.rule_not_greater(recall(), v)); break;
		case op::add: accept(out.rule_addition(recall(), v)); break;
		case op::subtract: accept(out.rule_subtraction(recall(), v)); break;
		case op::disjoin: accept(out.rule_or(recall(), v)); break;
		case op::exclude: accept(out.rule_xor(recall(), v)); break;
		case op::range: accept(out.rule_range(recall(), v)); break;
		case op::multiply: accept(out.rule_multiplication(recall(), v)); break;
		case op::divide: accept(out.rule_division(recall(), v)); break;
		case op::modulo: accept(out.rule_modulo(recall(), v)); break;
		case op::shift_left: accept(out.rule_shift_left(recall(), v)); break;
		case op::shift_right: accept(out.rule_shift_right(recall(), v)); break;
		case op::conjoin: accept(out.rule_and(recall(), v)); break;
		case op::negate: accept(out.rule_negate(v)); break;
		case op::complement: accept(out.rule_complement(v)); break;
		case op::subscript: accept(out.rule_subscript(recall(), v)); break;
		case op::lookup: accept(out.rule_lookup(recall(), v)); break;
		default: err.parser_unimplemented(tk.loc); break;
	}
}

