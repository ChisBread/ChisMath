#include "ChisExpr.h"
#include <iostream>
using namespace chis;
int main() {
	while(1) {
		std::string exp("x^(x^(x^(x^(x^(x^x)))))"), dep_var("x");
		std::cin >> exp >> dep_var;
		Expr expr(exp);
		//Expr expr("x^(21*x*9)");
		std::cout << expr.errors();
		expr.clear_errors();
		std::cout << "REV:" << expr.reverse_parse().string_expr() << std::endl;
		std::cout << "STD:" << expr.stdexpr().string_expr() << std::endl;
		//Çóµ¼
		expr = Expr::make_diff(expr, dep_var);
		std::cout << expr.errors();
		expr.clear_errors();
		if(expr.get_root()) {
			std::cout << "D:" << expr.reverse_parse().stdexpr().string_expr() << std::endl;
		}
	}
	//x^(1+y+1+y)
	//(1+x)*x*(x+1)
	return 0;
}