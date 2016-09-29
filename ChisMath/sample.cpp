#include "ChisExpr.h"
using namespace chis;
int main() {
	Expr expr1("min(y, x) + log(a, b)");
	Expr expr2(Expr::sin(expr1));
	Expr e2(expr1 ^ expr2);
	return 0;
}