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
		{ ID, 0 }, { CONST, 0 },
		{ EQU, 1 },
		{ ADD, 2 }, { SUB, 2 },
		{ MUL, 3 }, { DIV, 3 },
		{ MOD, 3 }, { POW, 3 },
		{ SIN, 4 }, { COS, 4 },
		{ TAN, 4 }, { COT, 4 },
		{ ARCSIN, 4 }, { ARCCOS, 4 },
		{ ARCTAN, 4 }, { ARCCOT, 4 },
		{ LOG, 4 }, { LN, 4 },
		{ DIFF, 4 },
		{ MIN, 4 }, { MAX, 4 },
	};
	int max_typeid = MAX;
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
			else if((*scan_index >= 'A' || *scan_index <= 'z') || (*scan_index == '_')) {
				++scan_index;
				for(; scan_index != end; ++scan_index) {
					//不是合法的ID字符
					if((*scan_index < 'A' || *scan_index > 'z')
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
	EQU_EXP
	-> ADD_EXP
	|  ADD_EXP = ADD_EXP

	ADD_EXP
	-> MUL_EXP
	|  ADD_EXP + MUL_EXP
	|  ADD_EXP - MUL_EXP

	MUL_EXP
	-> UNARY_EXP
	|  MUL_EXP * UNARY_EXP
	|  MUL_EXP / UNARY_EXP
	|  MUL_EXP % UNARY_EXP
	|  MUL_EXP ^ UNARY_EXP

	UNARY_EXP
	-> FUNC_EXP
	|  + UNARY_EXP
	|  - UNARY_EXP

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
		root = l = parse_unary();
		while(l && !lexer.is_end()) {
			if(lexer.lookahead()->type == MUL
				|| lexer.lookahead()->type == DIV
				|| lexer.lookahead()->type == MOD
				|| lexer.lookahead()->type == POW) {
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
				//throw("except '*','/','%','^'");
				//return nullptr;
				break;
			}
		}
		return root;
	}
	Expr::expr_node* Expr::expr_parser::parse_unary() {
		expr_node *root = nullptr;
		if(lexer.lookahead()->type == ADD
			|| lexer.lookahead()->type == SUB) {
			root = lexer.get_token();
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
					exp0 = parse_primary();
					if(lexer.lookahead()->type == DOT) {
						lexer.get_token();
					}
					else {
						//throw("except ','");
						root = nullptr;
						break;
					}
					exp1 = parse_primary();
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

}