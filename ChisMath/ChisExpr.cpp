#include "ChisExpr.h"
namespace chis {
	std::map<std::string, int> keyword = {
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
	int max_typeid = MAX;
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
				++scan_index;
				for(int ndot = 2; ndot && scan_index != end; ++scan_index) {
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
				token_buffer.push(expr_node(CONST, name));
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
				throw("WTF??");
			}
			break;
		}
		if(scan_index != end) {
			++scan_index;
		}
	}

	/*
	1
	EQU_EXP 
	-> ADD_EXP
	|  ADD_EXP = ADD_EXP
	2
	ADD_EXP
	-> MUL_EXP
	|  ADD_EXP + MUL_EXP
	|  ADD_EXP - MUL_EXP
	3
	MUL_EXP
	-> POW_EXP
	|  MUL_EXP * POW_EXP
	|  MUL_EXP / POW_EXP
	|  MUL_EXP % POW_EXP
	4
	POW_EXP
	-> UNARY_EXP
	| POW_EXP ^ UNARY_EXP
	5
	UNARY_EXP
	-> FUNC_EXP
	|  + UNARY_EXP
	|  - UNARY_EXP
	6
	FUNC_EXP
	-> PRIMARY_EXP
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
	PRIMARY_EXP
	-> ID
	|  NUM
	|  ( ADD_EXP )

	*/
	Expr::expr_node* Expr::expr_parser::parse_equ() {
		expr_node *l = nullptr, *root = nullptr, *r = nullptr;
		l = parse_add();
		if(l == nullptr) {
			//throw("WTF? except additive_exp before '='");
			return nullptr;
		}
		if(lexer.is_end()) { // additive_exp
			root = l;
		}
		else { // equ
			root = lexer.get_token();
			if(root->type != EQU) {
				//throw("WTF? except '=' ");
				root = nullptr;
			}
			else {
				r = parse_add();
				if(r == nullptr) {
					//throw("WTF? except additive_exp after '='");
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
					break;
				}
				root->insert_subtree(*r);
				l = root;
			}
			else {
				//throw("except '+'/'-'");
				//return nullptr;
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
			else {
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
		if(!lexer.is_end()) {
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
						//throw("except ','");
						root = nullptr;
						break;
					}
					exp1 = parse_add();
					if(lexer.lookahead()->type == RP) {
						lexer.get_token();
					}
					else {
						//throw("except ')'");
						root = nullptr;
						break;
					}
				}
				else {
					//throw("except '('");
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
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_primary() {
		expr_node *root = nullptr;
		if(!lexer.is_end()) {
			switch(lexer.lookahead()->type) {
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
					//throw("except ')'");
					root = nullptr;
				}
			}
				break;
			default:
				break;
			}
		}
		return root;
	}

	Expr Expr::reverse_parse(const expr_node *subroot) {
		//TODO 表达式化简
		switch(subroot->type) {
		case ID:
		case CONST:
			return Expr(subroot->name);
		case EQU:
			return
				equal(
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
		case COS:
		case TAN:
		case COT:
		case ARCSIN:
		case ARCCOS:
		case ARCTAN:
		case ARCCOT:
		case LN:
			return call_func(
				reverse_parse(subroot->subtree.front()),
				subroot->type, subroot->name);
		case LOG:
		case MAX:
		case MIN:
		case DIFF:
			return call_func(
				reverse_parse(subroot->subtree.front()),
				reverse_parse(subroot->subtree.back()),
				subroot->type, subroot->name);
		default:
			//TODO
			throw("fuck");
			break;
		}
	}
	Expr Expr::diff(const expr_node *subroot, const std::string &x) {
		//TODO
		switch(subroot->type) {
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
			Expr &&dq = diff(subroot->subtree.front(), x);
			Expr &&dp = diff(subroot->subtree.back(), x);
			return dq / p - (q * dp / (p ^ Expr("2")));
		}
		case POW:
		{
			Expr &&power = reverse_parse(subroot->subtree.back());
			Expr &&base = reverse_parse(subroot->subtree.front());
			//power中没有自变量x d(g(x)^c) = c*(g(x)^(c-1))*g'(x)dx
			if(power.id_type.find(x) == power.id_type.end()) {
				return power * (base ^ (power - Expr("1"))) * diff(subroot->subtree.front(), x);
			}
			else if(base.id_type.find(x) == base.id_type.end()) {
				return (base ^ power) * ln(base) * diff(subroot->subtree.back(), x);
			}
			// g^f->(e^ln(g))^f->e^(ln(g)*f)
			//d(g^f) -> e^(ln(g)*f)*d(ln(g)*f)
			else {
				Expr &&lng_x_f = ln(base)*power;
				return (Expr("e") ^ lng_x_f)*diff(lng_x_f.root, x);
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
			throw("fuck");
			break;
		}
	}

}