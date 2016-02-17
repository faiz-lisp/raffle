// Copyright (C) 2016 Mars Saxman. All rights reserved.
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided all source code distributions retain
// this paragraph and the above copyright notice. THIS SOFTWARE IS PROVIDED "AS
// IS" WITH NO EXPRESS OR IMPLIED WARRANTY.

#ifndef AST_H
#define AST_H

#include "location.h"
#include <string>
#include <memory>
#include <list>

namespace ast {

struct visitor;
struct node;
typedef std::unique_ptr<node> ptr;
struct delegate {
	virtual void ast_process(ptr &&) = 0;
	virtual void ast_done() = 0;
};

struct node {
	virtual ~node() {}
	virtual void accept(visitor&) = 0;
	virtual location loc() = 0;
};

struct number: public node {
	std::string text;
	number(std::string t, location l): text(t), tk_loc(l) {}
	virtual void accept(visitor &v) override;
	virtual location loc() override { return tk_loc; }
private:
	location tk_loc;
};

struct string: public node {
	std::string text;
	string(std::string t, location l): text(t), tk_loc(l) {}
	virtual void accept(visitor &v) override;
	virtual location loc() override { return tk_loc; }
private:
	location tk_loc;
};

struct symbol: public node {
	std::string text;
	symbol(std::string t, location l);
	virtual void accept(visitor &v) override;
	virtual location loc() override;
private:
	location tk_loc;
};

struct wildcard: public node {
	wildcard(location l): tk_loc(l) {}
	virtual void accept(visitor &v) override;
	virtual location loc() override { return tk_loc; }
private:
	location tk_loc;
};

struct invocation: public node {
	typedef enum {
		subscript, lookup
	} opcode;
	opcode id;
	ptr target;
	ptr argument;
	invocation(opcode o, ptr &&t, ptr &&a);
	virtual void accept(visitor &v) override;
	virtual location loc() override;
};

struct assign: public node {
	ptr sym;
	ptr exp;
	assign(ptr &&s, ptr &&e);
	virtual void accept(visitor &v) override;
	virtual location loc() override { return sym->loc() + exp->loc(); }
};

struct capture: public node {
	ptr sym;
	ptr exp;
	capture(ptr &&s, ptr &&e);
	virtual void accept(visitor &v) override;
	virtual location loc() override { return sym->loc() + exp->loc(); }
};

struct define: public node {
	ptr sym;
	ptr exp;
	define(ptr &&s, ptr &&e);
	virtual void accept(visitor &v) override;
	virtual location loc() override { return sym->loc() + exp->loc(); }
};

struct binary: public node {
	ptr left;
	ptr right;
	binary(ptr &&l, ptr &&r);
	virtual location loc() override { return left->loc() + right->loc(); }
};

struct arithmetic: public binary {
	typedef enum {
		add, subtract, multiply, divide, modulo, shift_left, shift_right,
	} opcode;
	opcode id;
	arithmetic(opcode o, ptr &&l, ptr &&r);
	virtual void accept(visitor &v) override;
};

struct logic: public binary {
	typedef enum {
		conjoin, disjoin, exclude,
	} opcode;
	opcode id;
	logic(opcode o, ptr &&l, ptr &&r);
	virtual void accept(visitor &v) override;
};

struct relation: public binary {
	typedef enum {
		equal, greater, lesser, not_equal, not_greater, not_lesser,
	} opcode;
	opcode id;
	relation(opcode o, ptr &&l, ptr &&r);
	virtual void accept(visitor &v) override;
};

struct range: public binary {
	range(ptr &&l, ptr &&r);
	virtual void accept(visitor &v) override;
};

struct invert: public node {
	typedef enum {
		negate, complement,
	} opcode;
	opcode id;
	ptr source;
	invert(opcode o, ptr &&s, location l);
	virtual location loc() override { return tk_loc + source->loc(); }
	virtual void accept(visitor &v) override;
private:
	location tk_loc;
};

struct tuple: public binary {
	tuple(ptr &&l, ptr &&r);
	virtual void accept(visitor &v) override;
};

struct constructor: public node {
	typedef enum {
		tuple, list, object
	} opcode;
	opcode id;
	std::list<ptr> items;
	constructor(opcode o, std::list<ptr> &&i, location l);
	virtual location loc() override { return tk_loc; }
	virtual void accept(visitor &v) override;
private:
	location tk_loc;
};

struct visitor {
	virtual void visit(number&) = 0;
	virtual void visit(string&) = 0;
	virtual void visit(symbol&) = 0;
	virtual void visit(wildcard&) = 0;
	virtual void visit(invocation&) = 0;
	virtual void visit(assign&) = 0;
	virtual void visit(capture&) = 0;
	virtual void visit(define&) = 0;
	virtual void visit(arithmetic&) = 0;
	virtual void visit(logic&) = 0;
	virtual void visit(relation&) = 0;
	virtual void visit(range&) = 0;
	virtual void visit(invert&) = 0;
	virtual void visit(tuple&) = 0;
	virtual void visit(constructor&) = 0;
};

} // namespace ast

#endif //AST_H

