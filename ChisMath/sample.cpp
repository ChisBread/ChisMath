#include "ChisExpr.h"
#include <iostream>
using namespace chis;
int main() {
	while(1) {
		std::string exp, dep_var;
		std::cin >> exp;
		Expr expr2(Expr::make_diff(Expr(exp), "x"));
		std::cout << expr2.string_expr() << std::endl;
	}
	return 0;
}