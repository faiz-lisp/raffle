// Copyright (C) 2016 Mars Saxman. All rights reserved.
// Permission is granted to use at your own risk and distribute this software
// in source and binary forms provided all source code distributions retain
// this paragraph and the above copyright notice. THIS SOFTWARE IS PROVIDED "AS
// IS" WITH NO EXPRESS OR IMPLIED WARRANTY.

#include "scanner.h"

scanner::scanner(): filter<char, chartype>()
{
	// classify each possible input byte
	//	error,
	//	space,
	//	punct,
	//	alpha,
	//	digit,
	//	newln,
	//	other
}

chartype scanner::apply(char in)
{
}

