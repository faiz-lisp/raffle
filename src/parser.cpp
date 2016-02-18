// Copyright (C) 2016 Mars Saxman. All rights reserved.
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided all source code distributions retain
// this paragraph and the above copyright notice. THIS SOFTWARE IS PROVIDED "AS
// IS" WITH NO EXPRESS OR IMPLIED WARRANTY.

#include "parser.h"
#include <assert.h>

bool parser::rightassoc(precedence x) {
	switch (x) {
		case precedence::statement: return true;
		case precedence::tuple: return true;
		case precedence::definition: return false;
		case precedence::relation: return false;
		case precedence::additive: return false;
		case precedence::multiplicative: return false;
		case precedence::negation: return true;
		case precedence::primary: return false;
	}
}

void parser::token_eof(location l) {
	if (!accept_delim(l, state::delim::file)) return;
	if (context.exp) {
		out.ast_process(cur());
		context.exp.reset();
	}
	out.ast_done();
}

void parser::token_number(location l, std::string text) {
	if (!accept_term(l)) return;
	emit(new ast::number(text, l));
}

void parser::token_symbol(location l, std::string text) {
	if (!accept_term(l)) return;
	emit(new ast::symbol(text, l));
}

void parser::token_string(location l, std::string text) {
	if (!accept_term(l)) return;
	emit(new ast::string(text, l));
}

void parser::token_underscore(location l) {
	if (!accept_term(l)) return;
	emit(new ast::wildcard(l));
}

void parser::token_l_paren(location l) {
	if (!accept_term(l)) return;
	open(l, state::delim::paren);
}

void parser::token_r_paren(location r) {
	if (!accept_delim(r, state::delim::paren)) return;
	close(ast::group::value, r);
}

void parser::token_l_bracket(location l) {
	if (!accept_term(l)) return;
	open(l, state::delim::bracket);
}

void parser::token_r_bracket(location r) {
	if (!accept_delim(r, state::delim::bracket)) return;
	close(ast::group::construct, r);
}

void parser::token_l_brace(location l) {
	if (!accept_term(l)) return;
	open(l, state::delim::brace);
}

void parser::token_r_brace(location r) {
	if (!accept_delim(r, state::delim::brace)) return;
	close(ast::group::scope, r);
}

void parser::token_comma(location l) {
	infix({precedence::tuple, l, [this]() {
		emit(new ast::tuple(pop(), cur()));
	}});
}

void parser::token_colon(location l) {
	infix({precedence::definition, l, [this]() {
		emit(new ast::define(pop(), cur()));
	}});
}

void parser::token_double_colon(location l) {
	err.parser_unexpected(l);
}

void parser::token_double_colon_equal(location l) {
	err.parser_unexpected(l);
}

void parser::token_semicolon(location l) {
	if (!accept_infix(l)) return;
	commit_all(l);
	if (context.grouping == state::delim::file) {
		// Top-level statements should be reported to the delegate.
		out.ast_process(cur());
		context.exp.reset();
	} else {
		// Statements in nested scopes are queued for later use.
		context.items.push_back(cur());
	}
}

void parser::token_dot(location l) {
	infix({precedence::primary, l, [this]() {
		emit(new ast::compose(pop(), cur()));
	}});
}

void parser::token_dot_dot(location l) {
	infix({precedence::additive, l, [this]() {
		emit(new ast::range(pop(), cur()));
	}});
}

void parser::token_l_arrow(location l) {
	infix({precedence::statement, l, [this]() {
		emit(new ast::assign(pop(), cur()));
	}});
}

void parser::token_r_arrow(location l) {
	infix({precedence::statement, l, [this]() {
		emit(new ast::capture(pop(), cur()));
	}});
}

void parser::token_plus(location l) {
	infix({precedence::additive, l, [this]() {
		emit(new ast::operate(ast::operate::add, pop(), cur()));
	}});
}

void parser::token_hyphen(location l) {
	if (expecting_term()) {
		prefix({precedence::negation, l, [this, l]() {
			emit(new ast::negate(ast::negate::numeric, cur(), l));
		}});
	} else {
		infix({precedence::additive, l, [this]() {
			emit(new ast::operate(ast::operate::sub, pop(), cur()));
		}});
	}
}

void parser::token_star(location l) {
	infix({precedence::multiplicative, l, [this]() {
		emit(new ast::operate(ast::operate::mul, pop(), cur()));
	}});
}

void parser::token_slash(location l) {
	infix({precedence::multiplicative, l, [this]() {
		emit(new ast::operate(ast::operate::div, pop(), cur()));
	}});
}

void parser::token_percent(location l) {
	infix({precedence::multiplicative, l, [this]() {
		emit(new ast::operate(ast::operate::rem, pop(), cur()));
	}});
}

void parser::token_equal(location l) {
	infix({precedence::relation, l, [this]() {
		emit(new ast::operate(ast::operate::eq, pop(), cur()));
	}});
}

void parser::token_tilde(location l) {
	err.parser_unexpected(l);
}

void parser::token_l_angle(location l) {
	infix({precedence::relation, l, [this]() {
		emit(new ast::operate(ast::operate::lt, pop(), cur()));
	}});
}

void parser::token_r_angle(location l) {
	infix({precedence::relation, l, [this]() {
		emit(new ast::operate(ast::operate::gt, pop(), cur()));
	}});
}

void parser::token_bang(location l) {
	prefix({precedence::negation, l, [this, l]() {
		emit(new ast::negate(ast::negate::logical, cur(), l));
	}});
}

void parser::token_bang_equal(location l) {
	infix({precedence::relation, l, [this]() {
		emit(new ast::operate(ast::operate::neq, pop(), cur()));
	}});
}

void parser::token_bang_tilde(location l) {
	err.parser_unexpected(l);
}

void parser::token_l_bangle(location l) {
	infix({precedence::relation, l, [this]() {
		emit(new ast::operate(ast::operate::nlt, pop(), cur()));
	}});
}

void parser::token_r_bangle(location l) {
	infix({precedence::relation, l, [this]() {
		emit(new ast::operate(ast::operate::ngt, pop(), cur()));
	}});
}

void parser::token_ampersand(location l) {
	infix({precedence::multiplicative, l, [this]() {
		emit(new ast::operate(ast::operate::conjoin, pop(), cur()));
	}});
}

void parser::token_pipe(location l) {
	infix({precedence::additive, l, [this]() {
		emit(new ast::operate(ast::operate::disjoin, pop(), cur()));
	}});
}

void parser::token_caret(location l) {
	infix({precedence::additive, l, [this]() {
		emit(new ast::operate(ast::operate::exclude, pop(), cur()));
	}});
}

void parser::token_l_guillemet(location l) {
	infix({precedence::multiplicative, l, [this]() {
		emit(new ast::operate(ast::operate::shl, pop(), cur()));
	}});
}

void parser::token_r_guillemet(location l) {
	infix({precedence::multiplicative, l, [this]() {
		emit(new ast::operate(ast::operate::shr, pop(), cur()));
	}});
}

void parser::prefix(oprec op) {
	if (!accept_prefix(op.loc)) return;
	context.ops.push(op);
}

void parser::infix(oprec op) {
	if (!accept_infix(op.loc)) return;
	if (rightassoc(op.prec)) {
		while (!context.ops.empty() && op.prec < context.ops.top().prec) {
			commit_next();
		}
	} else {
		while (!context.ops.empty() && op.prec <= context.ops.top().prec) {
			commit_next();
		}
	}
	context.vals.push(std::move(context.exp));
	context.ops.push(op);
}

void parser::commit_next() {
	context.ops.top().commit();
	context.ops.pop();
}

void parser::commit_all(location l) {
	if (expecting_term() && !context.ops.empty()) {
		err.parser_missing_right_operand(l);
		emit(new ast::wildcard(l));
	}
	while (!context.ops.empty()) {
		commit_next();
	}
	assert(context.vals.empty());
}

void parser::open(location l, state::delim g) {
	outer.push(std::move(context));
	context.startloc = l;
	context.grouping = g;
}

void parser::close(ast::group::opcode c, location r) {
	if (context.exp) {
		context.items.push_back(std::move(context.exp));
	}
	std::list<ast::ptr> result = std::move(context.items);
	location l = context.startloc + r;
	context = std::move(outer.top());
	outer.pop();
	assert(!context.exp);
	emit(new ast::group(c, std::move(result), l));
}

bool parser::accept_delim(location l, state::delim g) {
	if (context.grouping != g) {
		err.parser_mismatched_group(l);
		return false;
	}
	commit_all(l);
	return true;
}

bool parser::expecting_term() {
	return !context.exp;
}

bool parser::accept_term(location l) {
	if (!expecting_term()) {
		infix({precedence::primary, l, [this](){
			emit(new ast::apply(pop(), cur()));
		}});
	}
	return true;
}

bool parser::accept_prefix(location l) {
	if (expecting_term()) {
		return true;
	} else {
		err.parser_unexpected(l);
		return false;
	}
}

bool parser::accept_infix(location l) {
	if (expecting_term()) {
		err.parser_missing_left_operand(l);
		return false;
	} else {
		return true;
	}
}

void parser::emit(ast::node *n) {
	assert(!context.exp);
	context.exp.reset(n);
}

ast::ptr parser::pop() {
	assert(!context.vals.empty());
	ast::ptr n = std::move(context.vals.top());
	context.vals.pop();
	return n;
}

ast::ptr parser::cur() {
	assert(context.exp);
	return std::move(context.exp);
}
