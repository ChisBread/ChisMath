#include "ChisExpr.h"
namespace chis {
	std::map<std::string, int> keyword = {
		{ "ERROR", ERROR},
		{ "sin", SIN }, { "cos", COS },
		{ "tan", TAN }, { "cot", COT },
		{ "arcsin", ARCSIN }, { "arccos", ARCCOS },
		{ "arctan", ARCTAN }, { "arccot", ARCCOT },
		{ "log", LOG }, { "ln", LN },
		{ "pi", ID }, { "e", ID },
		{ "diff", DIFF },
		{ "min", MIN }, { "max", MAX },
	};
	std::map<int, int> prec_map = {
		{ ID, 7 }, { CONST, 7 },
		{ EQU, 1 },
		{ ADD, 2 }, { SUB, 2 },
		{ MUL, 3 }, { DIV, 3 },
		{ MOD, 3 }, 
		{ POW, 4 },
		{ NEGA, 5 }, { POSI, 5 },
		{ SIN, 6 }, { COS, 6 },
		{ TAN, 6 }, { COT, 6 },
		{ ARCSIN, 6 }, { ARCCOS, 6 },
		{ ARCTAN, 6 }, { ARCCOT, 6 },
		{ LOG, 6 }, { LN, 6 },
		{ DIFF, 6 },
		{ MIN, 6 }, { MAX, 6 },
	};
	std::map<int, bool> exchangadble = {
		{ ADD, true }, { SUB, false },
		{ MUL, true }, { DIV, false },
		{ MOD, false },
		{ POW, false },
	};
	int max_typeid = MAX;
	std::string Expr::error_message;
	Expr::expr_node Expr::ERROR_NODE(ERROR, "ERROR");
	Expr::expr_node Expr::NIL(CONST, "0");
	double to_double(const std::string &num) {
		std::strstream strs;
		strs << num;
		double ret;
		strs >> ret;
		return ret;
	}
	std::string to_string(double num) {
		std::strstream strs;
		strs << num;
		std::string ret;
		strs >> ret;
		return ret;
	}
	void Expr::expr_lexer::scan() {
		std::string name;
	BEGIN:
		name.push_back(*scan_index);
		switch(*scan_index) {
		case '=':
			token_buffer.push(expr_node(EQU, name));
			break;
		case '+':
			token_buffer.push(expr_node(ADD, name));
			break;
		case '-':
			token_buffer.push(expr_node(SUB, name));
			break;
		case '*':
			token_buffer.push(expr_node(MUL, name));
			break;
		case '/':
			token_buffer.push(expr_node(DIV, name));
			break;
		case '%':
			token_buffer.push(expr_node(MOD, name));
			break;
		case '^':
			token_buffer.push(expr_node(POW, name));
			break;
		case '(':
			token_buffer.push(expr_node(LP, name));
			break;
		case ')':
			token_buffer.push(expr_node(RP, name));
			break;
		case ',':
			token_buffer.push(expr_node(DOT, name));
			break;
		case ' ':
			++scan_index;
			name.pop_back();
			if(scan_index != end) {
				goto BEGIN;
			}
			break;
		default:
			if(*scan_index >= '0' && *scan_index <= '9' || (*scan_index == '.')) {
				int ndot = 2;
				if(*scan_index == '.') {
					--ndot;
				}
				++scan_index;
				for(; ndot && scan_index != end; ++scan_index) {
					if(*scan_index == '.') {
						--ndot;
					}
					//不是合法的NUM字符
					if(!ndot ||( (*scan_index < '0' || *scan_index > '9') && (*scan_index != '.'))) {
						--scan_index;
						break;
					}
					name.push_back(*scan_index);
				}
				if(name == ".") {
					error_message += "'.' is not a CONST.\n";
					token_buffer.push(ERROR_NODE);
				}
				else {
					token_buffer.push(expr_node(CONST, name));
				}
			}
			else if(
				(*scan_index >= 'A' && *scan_index <= 'Z') 
				|| (*scan_index >= 'a' && *scan_index <= 'z') 
				|| (*scan_index == '_')) {
				++scan_index;
				for(; scan_index != end; ++scan_index) {
					//不是合法的ID字符
					if((*scan_index < 'A' || *scan_index > 'Z') && (*scan_index < 'a' || *scan_index > 'z')
						&& (*scan_index < '0' || *scan_index > '9') && (*scan_index != '_')) {
						--scan_index;
						break;
					}
					name.push_back(*scan_index);
					if(keyword.find(name) != keyword.end()) {
						break;
					}
				}
				if(keyword.find(name) != keyword.end()) {
					token_buffer.push(expr_node(keyword[name], name));
				}
				else {
					token_buffer.push(expr_node(ID, name));
				}
			}
			else {
				error_message += name + "is unkown symbol.\n";
				token_buffer.push(ERROR_NODE);
			}
			break;
		}
		if(scan_index != end) {
			++scan_index;
		}
	}

	/*
	1
	EQU_EXP -> ADD_EXP
	|  ADD_EXP = ADD_EXP
	2
	ADD_EXP -> MUL_EXP
	|  ADD_EXP + MUL_EXP
	|  ADD_EXP - MUL_EXP
	3
	MUL_EXP -> POW_EXP
	|  MUL_EXP * POW_EXP
	|  MUL_EXP / POW_EXP
	|  MUL_EXP % POW_EXP
	4
	POW_EXP -> UNARY_EXP
	| POW_EXP ^ UNARY_EXP
	5
	UNARY_EXP -> FUNC_EXP
	|  + UNARY_EXP
	|  - UNARY_EXP
	6
	FUNC_EXP -> PRIMARY_EXP
	|  sin PRIMARY_EXP
	|  cos PRIMARY_EXP
	|  tan PRIMARY_EXP
	|  cot PRIMARY_EXP
	|  arcsin PRIMARY_EXP
	|  arccos PRIMARY_EXP
	|  arctan PRIMARY_EXP
	|  arccot PRIMARY_EXP
	|  ln PRIMARY_EXP
	|  diff ( ADD_EXP, ADD_EXP )
	|  log ( ADD_EXP, ADD_EXP )
	|  max ( ADD_EXP, ADD_EXP )
	|  min ( ADD_EXP, ADD_EXP )
	7
	PRIMARY_EXP -> ID
	|  NUM
	|  ( ADD_EXP )

	*/
	Expr::expr_node* Expr::expr_parser::parse_equ() {
		expr_node *l = nullptr, *root = nullptr, *r = nullptr;
		l = parse_add();
		if(l == nullptr) {
			return nullptr;
		}
		if(lexer.is_end()) { // additive_exp
			root = l;
		}
		else { // equ
			if(lexer.lookahead()->type != EQU) {
				error_message +=
					"except '=' after " + lexer.lookback()->name +
					" but got " + lexer.lookahead()->name + "\n";
				root = nullptr;
			}
			else {
				root = lexer.get_token();
				r = parse_add();
				if(r == nullptr) {
					root = nullptr;
				}
				else {
					root->insert_subtree(*l);
					root->insert_subtree(*r);
				}
			}
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_add() {
		//mul +/- mul +/- ... +/- mul
		expr_node *l = nullptr, *root = nullptr;
		root = l = parse_mul();
		while(l && !lexer.is_end()) {
			if(lexer.lookahead()->type == ADD 
				|| lexer.lookahead()->type == SUB) {
				root = lexer.get_token();
				root->insert_subtree(*l);
				expr_node *r = nullptr;
				r = parse_mul();
				if(r == nullptr) {
					root = nullptr;
					break;
				}
				root->insert_subtree(*r);
				l = root;
			}
			else {
				break;
			}
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_mul() {
		expr_node *l = nullptr, *root = nullptr;
		root = l = parse_pow();
		while(l && !lexer.is_end()) {
			if(lexer.lookahead()->type == MUL
				|| lexer.lookahead()->type == DIV
				|| lexer.lookahead()->type == MOD) {
				root = lexer.get_token();
				root->insert_subtree(*l);
				expr_node *r = nullptr;
				r = parse_pow();
				if(r == nullptr) {
					root = nullptr;
					break;
				}
				root->insert_subtree(*r);
				l = root;
			}
			else {
				break;
			}
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_pow() {
		expr_node *l = nullptr, *root = nullptr;
		root = l = parse_unary();
		while(l && !lexer.is_end()) {
			if(lexer.lookahead()->type == POW) {
				root = lexer.get_token();
				root->insert_subtree(*l);
				expr_node *r = nullptr;
				r = parse_unary();
				if(r == nullptr) {
					root = nullptr;
					break;
				}
				root->insert_subtree(*r);
				l = root;
			}
			else {
				break;
			}
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_unary() {
		expr_node *root = nullptr;
		if(lexer.lookahead()->type == ADD
			|| lexer.lookahead()->type == SUB
			|| lexer.lookahead()->type == NEGA
			|| lexer.lookahead()->type == POSI) {
			root = lexer.get_token();
			//更改属性
			if(root->type == SUB) {
				root->type = NEGA;
			}
			else if(root->type == ADD){
				root->type = POSI;
			}
			expr_node *l = nullptr;
			l = parse_unary();
			if(l == nullptr) {
				root = nullptr;
			}
			else {
				root->insert_subtree(*l);
			}
		}
		else {
			root = parse_func();
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_func() {
		expr_node *root = nullptr;
		switch(lexer.lookahead()->type) {
		case SIN:
		case COS:
		case TAN:
		case COT:
		case ARCSIN:
		case ARCCOS:
		case ARCTAN:
		case ARCCOT:
		case LN:
		{
			root = lexer.get_token();
			expr_node *l = parse_primary();
			if(l == nullptr) {
				root = nullptr;
			}
			else {
				root->insert_subtree(*l);
			}
		}
		break;
		case DIFF:
		case LOG:
		case MIN:
		case MAX:
		{
			root = lexer.get_token();
			expr_node *exp0 = nullptr, *exp1 = nullptr;
			if(lexer.lookahead()->type == LP) {
				lexer.get_token();
				exp0 = parse_add();
				if(lexer.lookahead()->type == DOT) {
					lexer.get_token();
				}
				else {
					error_message +=
						"except ',' after " + lexer.lookback()->name +
						" but got " + lexer.lookahead()->name + "\n";
					root = nullptr;
					break;
				}
				exp1 = parse_add();
				if(lexer.lookahead()->type == RP) {
					lexer.get_token();
				}
				else {
					error_message +=
						"except ')' after " + lexer.lookback()->name +
						" but got " + lexer.lookahead()->name + "\n";
					root = nullptr;
					break;
				}
			}
			else {
				error_message +=
					"except '(' after " + lexer.lookback()->name +
					" but got " + lexer.lookahead()->name + "\n";
				root = nullptr;
			}
			if(exp0 == nullptr) {
				root = nullptr;
				break;
			}
			root->insert_subtree(*exp0);
			if(exp1 == nullptr) {
				root = nullptr;
				break;
			}
			root->insert_subtree(*exp1);

		}
		break;
		default:
			root = parse_primary();
			break;
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_primary() {
		expr_node *root = nullptr;
		if(!lexer.is_end()) {
			switch(lexer.lookahead()->type) {
			case ERROR:
				error_message +=
					"except ID or CONST or (additive_exp) after " 
					+ lexer.lookback()->name + " but got ERROR\n";
				break;
			case ID:
			case CONST:
			{
				root = lexer.get_token();
			}
				break;
			case LP:
			{
				lexer.get_token();
				root = parse_add();
				if(lexer.lookahead()->type == RP) {
					lexer.get_token();
				}
				else {
					error_message += 
						"except ')' after " + lexer.lookback()->name 
						+ " but got " + lexer.lookahead()->name + "\n";
					//throw("except ')'");
					root = nullptr;
				}
			}
				break;
			default:
				error_message +=
					"except ID or CONST or '(' after " + lexer.lookback()->name +
					" but got " + lexer.lookahead()->name + "\n";
				break;
			}
		}
		else if(lexer.lookback()->name != "ERROR") {
			error_message +=
				"except ID or CONST or (additive_exp) after " + lexer.lookback()->name + "\n";
		}
		return root;
	}

	Expr Expr::reverse_parse(const expr_node *subroot) {
		if(!subroot) {
			return Expr("ERROR");
		}
		switch(subroot->type) {
		case ERROR:
			return Expr("ERROR");
		case ID:
		case CONST:
			return Expr(subroot->name);
		case EQU:
			return
				make_equal(
				reverse_parse(subroot->subtree.front())
				, reverse_parse(subroot->subtree.back()));
		case ADD:
			return
				reverse_parse(subroot->subtree.front())
				+ reverse_parse(subroot->subtree.back());
		case SUB:
			return
				reverse_parse(subroot->subtree.front())
				- reverse_parse(subroot->subtree.back());
		case MUL:
			return
				reverse_parse(subroot->subtree.front())
				* reverse_parse(subroot->subtree.back());
		case DIV:
			return
				reverse_parse(subroot->subtree.front())
				/ reverse_parse(subroot->subtree.back());
		case MOD:
			return
				reverse_parse(subroot->subtree.front())
				% reverse_parse(subroot->subtree.back());
		case POW:
			return
				reverse_parse(subroot->subtree.front())
				^ reverse_parse(subroot->subtree.back());
		case NEGA:
			return nega(reverse_parse(subroot->subtree.front()));
		case POSI:
			return reverse_parse(subroot->subtree.front());
		case SIN:
			return sin(reverse_parse(subroot->subtree.front()));
		case COS:
			return cos(reverse_parse(subroot->subtree.front()));
		case TAN:
			return tan(reverse_parse(subroot->subtree.front()));
		case COT:
			return cot(reverse_parse(subroot->subtree.front()));
		case ARCSIN:
			return arcsin(reverse_parse(subroot->subtree.front()));
		case ARCCOS:
			return arccos(reverse_parse(subroot->subtree.front()));
		case ARCTAN:
			return arctan(reverse_parse(subroot->subtree.front()));
		case ARCCOT:
			return arccot(reverse_parse(subroot->subtree.front()));
		case LN:
			return ln(reverse_parse(subroot->subtree.front()));
		case LOG:
			return log(reverse_parse(subroot->subtree.front()),
				reverse_parse(subroot->subtree.back()));
		case MAX:
			return max(reverse_parse(subroot->subtree.front()),
				reverse_parse(subroot->subtree.back()));
		case MIN:
			return min(reverse_parse(subroot->subtree.front()),
				reverse_parse(subroot->subtree.back()));
		case DIFF:
			return call_func(
				reverse_parse(subroot->subtree.front()),
				reverse_parse(subroot->subtree.back()),
				subroot->type, subroot->name);
		default:
			error_message
				+= subroot->name + "is unkown symbol.\n";
			return Expr("ERROR");
			break;
		}
	}
	Expr Expr::diff(const expr_node *subroot, const std::string &x) {
		if(!subroot) {
			return Expr("ERROR");
		}
		switch(subroot->type) {
		case ERROR:
			return Expr("ERROR");
		case ID:
			if(subroot->name == x) {
				return Expr("1");
			}
		case CONST:
			return Expr("0");
		case NEGA:
			return nega(diff(subroot->subtree.front(), x));
		case POSI:
			return diff(subroot->subtree.front(), x);
		case EQU:
			return 
				make_equal(diff(subroot->subtree.front(), x), diff(subroot->subtree.back(), x));
		case ADD:
			return diff(subroot->subtree.front(), x) + diff(subroot->subtree.back(), x);
		case SUB:
			return diff(subroot->subtree.front(), x) - diff(subroot->subtree.back(), x);
		case MUL:
			return
				diff(subroot->subtree.front(), x) * reverse_parse(subroot->subtree.back())
				+ diff(subroot->subtree.back(), x) * reverse_parse(subroot->subtree.front());
		case DIV:
		{
			// q / p
			// dq / p - (q * dp / p^2))
			Expr &&q = reverse_parse(subroot->subtree.front());
			Expr &&p = reverse_parse(subroot->subtree.back());
			Expr p2(p);
			Expr &&dq = diff(subroot->subtree.front(), x);
			Expr &&dp = diff(subroot->subtree.back(), x);
			return std::move(dq) / std::move(p) 
				- (std::move(q) * std::move(dp) / (std::move(p2) ^ Expr("2")));
		}
		case POW:
		{
			Expr &&power = reverse_parse(subroot->subtree.back());
			Expr &&base = reverse_parse(subroot->subtree.front());
			//power中没有自变量x d(g(x)^c) = c*(g(x)^(c-1))*g'(x)dx
			if(power.id_type.find(x) == power.id_type.end()) {
				Expr pw2(power);
				return std::move(pw2) * (std::move(base) ^ (std::move(power) - Expr("1"))) * diff(subroot->subtree.front(), x);
			}
			else if(base.id_type.find(x) == base.id_type.end()) {
				Expr bs2(base);
				return (std::move(bs2) ^ std::move(power)) * ln(std::move(base)) * diff(subroot->subtree.back(), x);
			}
			// g^f->(e^ln(g))^f->e^(ln(g)*f)
			//d(g^f) -> e^(ln(g)*f)*d(ln(g)*f)
			else {
				Expr &&lng_x_f = ln(std::move(base)) * std::move(power);
				return (Expr("e") ^ Expr(lng_x_f))*diff(lng_x_f.root, x);
			}
		}
		case SIN:
			return
				cos(reverse_parse(subroot->subtree.front()))
				* diff(subroot->subtree.front(), x);
		case COS:
			return
				nega(
				sin(reverse_parse(subroot->subtree.front())))
				* diff(subroot->subtree.front(), x);
		case TAN:
			return
				(Expr("1")
				/ (
				cos(reverse_parse(subroot->subtree.front())) ^ Expr("2")
				))
				* diff(subroot->subtree.front(), x);
		case COT:
			return
				nega(
				(Expr("1")
				/ (
				sin(reverse_parse(subroot->subtree.front())) ^ Expr("2")
				)))
				* diff(subroot->subtree.front(), x);
		case ARCSIN:
			return
				Expr("1") /
				(
				(Expr("1") - ((reverse_parse(subroot->subtree.front())) ^ Expr("2")))
				^ Expr("0.5")
				)
				* diff(subroot->subtree.front(), x);
		case ARCCOS:
			return
				nega(Expr("1") /
				(
				(Expr("1") - ((reverse_parse(subroot->subtree.front())) ^ Expr("2")))
				^ Expr("0.5")
				))
				* diff(subroot->subtree.front(), x);
		case ARCTAN:
			return
				Expr("1") /
				(
				(Expr("1") + ((reverse_parse(subroot->subtree.front())) ^ Expr("2")))
				^ Expr("0.5")
				)
				* diff(subroot->subtree.front(), x);
		case ARCCOT:
			return
				nega(Expr("1") /
				(
				(Expr("1") + ((reverse_parse(subroot->subtree.front())) ^ Expr("2")))
				^ Expr("0.5")
				))
				* diff(subroot->subtree.front(), x);
		case LOG:
		{
			Expr &&lnx_div_lna = ln(reverse_parse(subroot->subtree.back()))
				/ ln(reverse_parse(subroot->subtree.front()));
			return diff(lnx_div_lna.root, x);
		}
		break;
		case LN:
			return Expr("1") / reverse_parse(subroot->subtree.front()) * diff(subroot->subtree.front(), x);
		default:
			error_message
				+= subroot->name + " is undifferentiable.\n";
			//throw("fuck");
			break;
		}
		return Expr("ERROR");
	}
	std::list<std::vector<std::pair<Expr*,int>>> Expr::stdexpr(const expr_node *st) {
		if(!st) {
			return{ { { new Expr("ERROR"), 1 } } };
		}
		auto count_to = 
			[](std::vector<std::pair<Expr*, int>> &a
			, const std::vector<std::pair<Expr*, int>> &b) {
			for(auto &i : b) {
				auto j = a.begin();
				for(; j != a.end(); ++j) {
					if(j->first == i.first) {
						j->second += i.second;
						break;
					}
				}
				if(j == a.end()) {
					a.push_back(i);
				}
			}
		};
		switch(st->type) {
		case ERROR:
			return{ { { new Expr("ERROR"), 1 } } };
		case ID:
		case CONST:
			return{ { { new  Expr(st->name), 1 } } };
		case ADD:
		{
			auto &&subexpr = stdexpr(st->subtree[0]);
			subexpr.splice(subexpr.begin(), stdexpr(st->subtree[1]));
			return std::move(subexpr);
		}
		case SUB:
		{
			auto &&expra = stdexpr(st->subtree[0]);
			auto &&exprb = stdexpr(st->subtree[1]);
			Expr *nega1 = new Expr("-1");
			for(auto &i:exprb) {
				i.push_back({ nega1 , 1});
			}
			expra.splice(expra.begin(), exprb);
			return std::move(expra);
		}
		case MUL:
		{
			auto &&expra = stdexpr(st->subtree[0]);
			auto &&exprb = stdexpr(st->subtree[1]);
			std::list<std::vector<std::pair<Expr*, int>>> ret;
			for(const auto &i: exprb) {
				for(auto j : expra) {
					count_to(j, i);
					ret.push_back(std::move(j));
				}
			}
			return std::move(ret);
		}
		case DIV:
		{
			auto &&expra = stdexpr(st->subtree[0]);
			auto &&exprb = stdexpr(st->subtree[1]);
			Expr *b = new Expr(to_expr(exprb) ^ Expr("-1"));
			for(auto &i : expra) {
				i.push_back({ b, 1});
			}
			return std::move(expra);
		} 
		case MOD:
		{
			auto &&expra = stdexpr(st->subtree[0]);
			auto &&exprb = stdexpr(st->subtree[1]);
			return{ { { new Expr(to_expr(expra) % to_expr(exprb)), 1 } } };
		}
		case POW:
		{
			auto &&expra = stdexpr(st->subtree[0]);
			auto &&exprb = stdexpr(st->subtree[1]);
			std::list<std::vector<std::pair<Expr*, int>>> ret;
			if(
				//(x1+x2...+xn)^m
				expra.size() > 1
				//exprb为整数
				&& exprb.size() == 1 && exprb.front().size() == 1
				&& exprb.front().front().first->root->type == CONST
				&& exprb.front().front().first->root->name.find('.') == std::string::npos
				&& exprb.front().front().first->root->name.find('-') == std::string::npos) {
				int n = to_double(exprb.front().front().first->root->name);
				ret = expra;
				//去括号
				for(int k = 1; k < n; ++k) {
					std::list<std::vector<std::pair<Expr*, int>>> temp;
					for(const auto &i : expra) { 
						for(auto j : ret) {
							count_to(j, i);
							temp.push_back(std::move(j));
						}
					}
					ret.swap(temp);
				}
			}
			else {
				ret.push_back({ { new Expr(to_expr(expra) ^ to_expr(exprb)), 1 } });
			}
			return std::move(ret);
		}
		case NEGA:
		{
			auto &&expr = stdexpr(st->subtree[0]);
			Expr *nega1 = new Expr("-1");
			for(auto &i : expr) {
				i.push_back({ nega1 , 1});
			}
			return expr;
		}
		case POSI:
			return 	stdexpr(st->subtree[0]);
		case SIN:
		case COS:
		case TAN:
		case COT:
		case ARCSIN:
		case ARCCOS:
		case ARCTAN:
		case ARCCOT:
		case LN:
		{
			auto &&expr = stdexpr(st->subtree[0]);
			Expr a(to_expr(expr));

			return{ { { new Expr(call_func(a, st->type, st->name)), 1 } } };
		}
		case LOG:
		case MAX:
		case MIN:
		case DIFF:
		{
			auto &&expra = stdexpr(st->subtree[0]);
			auto &&exprb = stdexpr(st->subtree[1]);

			Expr a(to_expr(expra));
			Expr b(to_expr(exprb));

			return{ { { new Expr(call_func(a, b, st->type, st->name)), 1 } } };
		}
		default:
			error_message
				+= st->name + "is unkown symbol.\n";
			break;
		}
		return{ { { new Expr("ERROR"), 1 } } };
	}

	Expr operator+(Expr &&a, Expr &&b) {
		// x + 0 = x
		if(a.root->name == "0") {
			return std::move(b);
		}
		else if(b.root->name == "0") {
			return std::move(a);
		}
		//c1 + c2 = c3
		if(a.root->type == CONST && b.root->type == CONST) {
			return
				to_string(
				to_double(a.root->name) + to_double(b.root->name));
		}
		//-c1 + c2 = c3
		if(a.root->type == NEGA
			&& a.root->subtree[0]->type == CONST && b.root->type == CONST) {
			return
				to_string(
				to_double(b.root->name) - to_double(a.root->subtree[0]->name));
		}
		//c1 + -c2 = c3
		if(b.root->type == NEGA
			&& b.root->subtree[0]->type == CONST && a.root->type == CONST) {
			return
				to_string(
				to_double(a.root->name) - to_double(b.root->subtree[0]->name));
		}
		//-c1 + -c2 = c3
		if(a.root->type == NEGA && b.root->type == NEGA
			&& a.root->subtree[0]->type == CONST
			&& b.root->subtree[0]->type == CONST) {
			return
				Expr::nega(
				to_string(
				to_double(a.root->subtree[0]->name) + to_double(b.root->subtree[0]->name)));
		}
		if(a.root->type == MUL && b.root->type == MUL) {
			//n*x + m*x = (m+n) * x
			if(a.root->subtree[0]->type == CONST
				&& b.root->subtree[0]->type == CONST
				&& Expr::equal(a.root->subtree[1], b.root->subtree[1])) {
				return Expr(to_string(
					to_double(a.root->subtree[0]->name)
					+ to_double(b.root->subtree[0]->name))) * Expr::reverse_parse(b.root->subtree[1]);
			}
			//TODO
		}
		//n*x + x = (n+1)*x
		if(a.root->type == MUL) {
			// *
			//n x
			if(a.root->subtree[0]->type == CONST &&
				Expr::equal(a.root->subtree[1], b.root)) {
				return Expr(to_string(
					to_double(a.root->subtree[0]->name) + 1)) * std::move(b);
			}
			// *
			//x n
			if(a.root->subtree[1]->type == CONST &&
				Expr::equal(a.root->subtree[0], b.root)) {
				return Expr(to_string(
					to_double(a.root->subtree[1]->name) + 1))* std::move(b);
			}
		}
		//x + n*x = (n+1)*x
		if(b.root->type == MUL) {
			// *
			//n x
			if(b.root->subtree[0]->type == CONST &&
				Expr::equal(b.root->subtree[1], a.root)) {
				return Expr(to_string(
					to_double(b.root->subtree[0]->name) + 1))*(std::move(a));
			}
			// *
			//x n
			if(b.root->subtree[1]->type == CONST &&
				Expr::equal(b.root->subtree[0], a.root)) {
				return Expr(to_string(
					to_double(b.root->subtree[1]->name) + 1))*(std::move(a));
			}
		}
		//x + x = 2x
		if(Expr::equal(a.root, b.root)) {
			return Expr("2") * std::move(b);
		}
		//x + -x = 0
		if(b.root->type == NEGA && Expr::equal(a.root, b.root->subtree[0])) {
			return Expr("0");
		}
		if(a.root->type == NEGA && Expr::equal(a.root->subtree[0], b.root)) {
			return Expr("0");
		}
		//x-y + y = x
		if(b.root->type == SUB && Expr::equal(a.root, b.root->subtree[1])) {
			return Expr::reverse_parse(b.root->subtree[0]);
		}
		if(a.root->type == SUB && Expr::equal(b.root, a.root->subtree[1])) {
			return Expr::reverse_parse(a.root->subtree[0]);
		}
		return Expr::opt(std::move(a), ADD, "+", std::move(b));
	}
	Expr operator-(Expr &&a, Expr &&b) {
		// 0 - x = -x
		// x - 0 = x
		if(a.root->name == "0") {
			return Expr::nega(std::move(b));
		}
		else if(b.root->name == "0") {
			return std::move(a);
		}
		//c1 - c2 = c3
		if(a.root->type == CONST && b.root->type == CONST) {
			return to_string(
				to_double(a.root->name) - to_double(b.root->name));
		}
		//-c1 - c2 = c3
		if(a.root->type == NEGA
			&& a.root->subtree[0]->type == CONST && b.root->type == CONST) {
			return
				Expr::nega(
				to_string(
				to_double(b.root->name) + to_double(a.root->subtree[0]->name)));
		}
		//x - x = 0
		else if(Expr::equal(a.root, b.root)) {
			return Expr("0");
		}
		if(a.root->type == MUL && b.root->type == MUL) {
			//n*x - m*x = (m-n) * x
			if(a.root->subtree[0]->type == CONST
				&& b.root->subtree[0]->type == CONST
				&& Expr::equal(a.root->subtree[1], b.root->subtree[1])) {
				Expr(to_string(
					to_double(a.root->subtree[0]->name)
					- to_double(b.root->subtree[0]->name)))*Expr::reverse_parse(b.root->subtree[1]);
			}
			//TODO
		}
		// x - -y = x + y
		if(b.root->type == NEGA) {
			return std::move(a) + Expr::reverse_parse(b.root->subtree[0]);
		}
		return Expr::opt(std::move(a), SUB, "-", std::move(b));
	}
	Expr operator*(Expr &&a, Expr &&b) {
		// x * 1 = x
		if(a.root->name == "1") {
			return std::move(b);
		}
		else if(b.root->name == "1") {
			return std::move(a);
		}
		if(a.root->name == "0" || b.root->name == "0") {
			return Expr("0");
		}
		//c1 * c2 = c3
		if(a.root->type == CONST && b.root->type == CONST) {
			return to_string(
				to_double(a.root->name) * to_double(b.root->name));
		}
		//x * x = x^2
		if(Expr::equal(a.root, b.root)) {
			return Expr::opt(std::move(a), POW, "^", Expr("2"));
		}
		//m*x * n = m*n*x
		if(a.root->type == MUL && a.root->subtree[0]->type == CONST
			&& b.root->type == CONST) {
			a.root->subtree[0]->name 
				= to_string(to_double(a.root->subtree[0]->name)*to_double(b.root->name));
			return std::move(a);
		}
		// (fx / gx)*gx = fx
		if(b.root->type == DIV && Expr::equal(a.root, b.root->subtree[1])) {
			return Expr::reverse_parse(b.root->subtree[0]);
		}
		if(a.root->type == DIV && Expr::equal(b.root, a.root->subtree[1])) {
			return Expr::reverse_parse(a.root->subtree[0]);
		}

		if(b.root->type == POW && a.root->type == POW) {
			// gx^a * gx^b = gx^(a+b)
			if(Expr::equal(b.root->subtree[0], a.root->subtree[0])) {
				return Expr::reverse_parse(b.root->subtree[0])
					^ (Expr::reverse_parse(a.root->subtree[1]) + Expr::reverse_parse(b.root->subtree[1]));
			}
			// gx^a * fx^a = (gx*fx)^a
			if(Expr::equal(b.root->subtree[1], a.root->subtree[1])) {
				return
					(Expr::reverse_parse(a.root->subtree[0])) * Expr::reverse_parse(b.root->subtree[0])
					^ Expr::reverse_parse(b.root->subtree[1]);
			}

		}
		// gx^a * gx = gx^(a+1)
		if(b.root->type == POW
			&& Expr::equal(b.root->subtree[0], a.root)) {
			return Expr::reverse_parse(b.root->subtree[0])
				^ (Expr::reverse_parse(b.root->subtree[1]) + Expr("1"));
		}
		if(a.root->type == POW
			&& Expr::equal(b.root, a.root->subtree[0])) {
			return Expr::reverse_parse(a.root->subtree[0])
				^ (Expr::reverse_parse(a.root->subtree[1]) + Expr("1"));
		}
		//-x * -y = x * y
		if(b.root->type == NEGA && a.root->type == NEGA) {
			return
				Expr::reverse_parse(a.root->subtree[0])
				* Expr::reverse_parse(b.root->subtree[0]);
		}
		// x * -y = -(x*y)
		if(b.root->type == NEGA) {
			return
				Expr::nega(std::move(a) * Expr::reverse_parse(b.root->subtree[0]));
		}
		if(a.root->type == NEGA) {
			return
				Expr::nega(Expr::reverse_parse(a.root->subtree[0]) * std::move(b));
		}
		return Expr::opt(std::move(a), MUL, "*", std::move(b));
	}
	Expr operator/(Expr &&a, Expr &&b) {
		// 0 / x = 0
		if(a.root->name == "0") {
			return Expr("0");
		}
		// x / 1 = x
		if(b.root->name == "1") {
			return std::move(a);
		}
		//c1 / c2 = c3
		if(a.root->type == CONST && b.root->type == CONST) {
			return
				to_string(
				to_double(a.root->name) / to_double(b.root->name));
		}
		//x / x = 1
		else if(Expr::equal(a.root, b.root)) {
			return Expr("1");
		}
		// gx^a / gx^b = gx^(a-b)
		if(b.root->type == POW && a.root->type == POW
			&& Expr::equal(b.root->subtree[0], a.root->subtree[0])) {
			return
				Expr::reverse_parse(b.root->subtree[0])
				^ (Expr::reverse_parse(a.root->subtree[1]) - Expr::reverse_parse(b.root->subtree[1]));
		}
		// x/y / z = x/(y*z)
		if(a.root->type == DIV) {
			return
				Expr::reverse_parse(a.root->subtree[0])
				/ (Expr::reverse_parse(a.root->subtree[1]) * std::move(b));
		}
		// x/y / x = 1/y
		if(a.root->type == DIV
			&& Expr::equal(a.root->subtree[0], b.root)) {
			return
				Expr("1") / Expr::reverse_parse(a.root->subtree[1]);
		}
		// x*y / x = y
		if(a.root->type == MUL) {
			if(Expr::equal(a.root->subtree[0], b.root)) {
				return Expr::reverse_parse(a.root->subtree[1]);
			}
			else if(Expr::equal(a.root->subtree[1], b.root)) {
				return Expr::reverse_parse(a.root->subtree[0]);
			}
		}
		// -a / -b = a / b
		if(b.root->type == NEGA && a.root->type == NEGA) {
			return
				Expr::reverse_parse(a.root->subtree[0])
				/ Expr::reverse_parse(b.root->subtree[0]);
		}
		return Expr::opt(std::move(a), DIV, "/", std::move(b));
	}
	Expr operator%(Expr &&a, Expr &&b) {
		return Expr::opt(std::move(a), MOD, "%", std::move(b));
	}
	Expr operator^(Expr &&a, Expr &&b) {
		// x ^ 1 = x
		if(b.root->name == "1") {
			return std::move(a);
		}
		// 1 ^ y = 1
		if(a.root->name == "1") {
			return std::move(a);
		}
		// x ^ 0 = 1
		if(b.root->name == "0") {
			return Expr("1");
		}
		// (x ^ g) ^ f -> x ^(g*f)
		if(a.root->type == POW) {
			return
				Expr::reverse_parse(a.root->subtree[0])
				^ (Expr::reverse_parse(a.root->subtree[1])* std::move(b));
		}
		// (-1)^n
		if(
			(b.root->type == CONST 
			|| (b.root->type == NEGA && b.root->subtree[0]->type == CONST))
			&& a.root->type == NEGA && a.root->subtree[0]->name == "1"
			&& b.root->name.find(".") == std::string::npos) {
			int n = b.root->type == CONST ? 
				(int)to_double(b.root->name) : (int)to_double(b.root->subtree[0]->name);
			return n % 2 ? std::move(a) : Expr("1");
		}
		return Expr::opt(std::move(a), POW, "^", std::move(b));
	}
}