#include "ChisExpr.h"
#include <iostream>
using namespace chis;
int main() {
	while(1) {
		std::string exp, dep_var;
		std::cin >> exp >> dep_var;
		Expr expr(exp);
		//Expr expr("x^(21*x*9)");
		std::cout << expr.errors();
		expr.clear_errors();
		std::cout << "REV:" << expr.reverse_parse().string_expr() << std::endl;
		std::cout << "STD:" << expr.standardization().string_expr() << std::endl;
		//Çóµ¼
		expr = Expr::make_diff(expr, dep_var);
		std::cout << expr.errors();
		expr.clear_errors();
		if(expr.get_root()) {
			std::cout << "D:" << expr.string_expr() << std::endl;
		}
		
	}
	//(1+x)*x*(x+1)
	return 0;
}