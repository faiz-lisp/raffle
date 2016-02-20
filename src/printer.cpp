// Copyright (C) 2016 Mars Saxman. All rights reserved.
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided all source code distributions retain
// this paragraph and the above copyright notice. THIS SOFTWARE IS PROVIDED "AS
// IS" WITH NO EXPRESS OR IMPLIED WARRANTY.

#include "printer.h"

using namespace ast;

void printer::visit(ast::number& n) {
	out << n.text;
}

void printer::visit(ast::string& n) {
	out << "\"" << n.text << "\"";
}

void printer::visit(ast::symbol& n) {
	out << n.text;
}

void printer::visit(ast::wildcard& n) {
	out << "_";
}

void printer::visit(negate& n) {
	switch (n.id) {
		case negate::numeric: out << "-"; break;
		case negate::logical: out << "!"; break;
	}
	n.source->accept(*this);
}

void printer::visit(ast::operate& n) {
	switch (n.id) {
		case operate::add: infix(n, " + "); break;
		case operate::sub: infix(n, " - "); break;
		case operate::mul: infix(n, " * "); break;
		case operate::div: infix(n, " / "); break;
		case operate::rem: infix(n, " % "); break;
		case operate::shl: infix(n, " << "); break;
		case operate::shr: infix(n, " >> "); break;
		case operate::conjoin: infix(n, " & "); break;
		case operate::disjoin: infix(n, " | "); break;
		case operate::exclude: infix(n, " ^ "); break;
		case operate::eq: infix(n, " = "); break;
		case operate::lt: infix(n, " < "); break;
		case operate::gt: infix(n, " > "); break;
		case operate::neq: infix(n, " != "); break;
		case operate::nlt: infix(n, " !< "); break;
		case operate::ngt: infix(n, " !> "); break;
	}
}

void printer::infix(binary &n, std::string t) {
	if (level++) out << "\xC2\xAB";
	n.left->accept(*this);
	out << t;
	n.right->accept(*this);
	if (--level) out << "\xC2\xBB";
}

void printer::group(std::string l, ast::group& n, std::string r) {
	out << l;
	printer sub(out);
	n.source->accept(sub);
	out << r;
}

