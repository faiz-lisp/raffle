// Copyright (C) 2016 Mars Saxman. All rights reserved.
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided all source code distributions retain
// this paragraph and the above copyright notice. THIS SOFTWARE IS PROVIDED "AS
// IS" WITH NO EXPRESS OR IMPLIED WARRANTY.

#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <stack>
#include <queue>

class parser: public lexer::output {
	// the classic shunting-yard algorithm
	std::stack<int> values;
	std::stack<int> states;
public:
	struct output {
		virtual int parse_number(std::string) = 0;
		virtual int parse_symbol(std::string) = 0;
		virtual int parse_string(std::string) = 0;
		virtual int parse_sequence(int, int) = 0;
		virtual int parse_list(int, int) = 0;
		virtual int parse_addition(int, int) = 0;
		virtual int parse_subtraction(int, int) = 0;
		virtual int parse_multiplication(int, int) = 0;
		virtual int parse_division(int, int) = 0;
		virtual int parse_modulo(int, int) = 0;
	};
	struct error {
		virtual void parse_unexpected(lexer::position) = 0;
		virtual void parse_mismatched_paren(lexer::position) = 0;
	};
	parser(output &o, error &e): out(o), err(e) {}
	virtual void token_number(lexer::position p, std::string text) override;
	virtual void token_symbol(lexer::position p, std::string text) override;
	virtual void token_string(lexer::position p, std::string text) override;
	virtual void token_paren_open(lexer::position) override;
	virtual void token_paren_close(lexer::position) override;
	virtual void token_bracket_open(lexer::position) override {}
	virtual void token_bracket_close(lexer::position) override {}
	virtual void token_brace_open(lexer::position) override {}
	virtual void token_brace_close(lexer::position) override {}
	virtual void token_comma(lexer::position) override;
	virtual void token_semicolon(lexer::position) override;
	virtual void token_plus(lexer::position) override;
	virtual void token_minus(lexer::position) override;
	virtual void token_star(lexer::position) override;
	virtual void token_slash(lexer::position) override;
	virtual void token_percent(lexer::position) override;
protected:
	void parse_infix(int);
	void commit_infix();
private:
	output &out;
	error &err;
};

#endif //PARSER_H

