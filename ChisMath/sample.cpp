#include "ChisExpr.h"
#include <iostream>
using namespace chis;
int main() {
	while(1) {
		std::string exp, dep_var;
		std::cin >> exp;
		Expr expr(Expr::make_diff(Expr(exp), "x"));
		//Expr expr("x^(21*x*9)");
		std::cout << expr.string_expr() << std::endl;
		
	}
	//(11*x^2+2*x^2)
	return 0;
}