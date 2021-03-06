#include "ChisExpr.h"
namespace chis {

	std::unordered_map<std::string, int> keyword = {
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
	const int max_typeid = MAX;
	class map {
	public:
		map(const std::initializer_list<std::pair<int, int >> &a) {
			for(auto &i : _map) {
				i = -1;
			}
			for(auto &i : a) {
				_map[i.first] = i.second;
			}
		}
		int find(int i) {
			if(i >= max_typeid) {
				return -1;
			}
			return _map[i];
		}
		int end() {
			return -1;
		}
		int& operator[](size_t i) {
			return _map[i];
		}
	private:
		int _map[max_typeid];
	};
	map prec_map = {
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
	map operand = {
		{ ADD, 2 }, { SUB, 2 },
		{ MUL, 2 }, { DIV, 2 },
		{ MOD, 2 }, { LOG, 2 }, 
		{ POW, 2 }, { MAX, 2 },
		{ MIN, 2 },
		{ NEGA, 1 }, { POSI, 1 },
		{ SIN, 1 }, { COS, 1 },
		{ TAN, 1 }, { COT, 1 },
		{ ARCSIN, 1 }, { ARCCOS, 1 },
		{ ARCTAN, 1 }, { ARCCOT, 1 },
	};
	map exchangadble = {
		{ ADD, true }, { SUB, false },
		{ MUL, true }, { DIV, false },
		{ MOD, false },
		{ POW, false },
	};
	
	std::string Expr::error_message;
	Expr::expr_node Expr::ERROR_NODE(ERROR, "ERROR");
	Expr::expr_node Expr::NIL(CONST, "0");
	FLOAT to_FLOAT(const std::string &num) {
		std::strstream strs;
		strs << num;
		FLOAT ret;
		strs >> ret;
		return ret;
	}
	std::string to_string(FLOAT num) {
		std::strstream strs;
		strs.setf(std::ios::fixed);
		strs << num;
		std::string ret;
		strs >> ret;
		if(ret.find('.') != std::string::npos) {
			while(!ret.empty() && 
				(ret.back() == '0' || ret.back() == '#'
				|| ret.back() == 'I' || ret.back() == 'N' || ret.back() == 'F')) {
				ret.pop_back();
			}
			if(ret.back() == '.') {
				ret.pop_back();
			}
			if(ret.empty()) {
				ret = "0";
			}
		}
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
					if(ndot != 2) {
						while(!name.empty() &&(name.back() == '0')) {
							name.pop_back();
						}
						if(name.back() == '.') {
							name.pop_back();
						}
						if(name.empty()) {
							name = "0";
						}

					}
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

	Expr::expr_node::expr_node(int type, std::string name) :type(type), name(name) {
		if(prec_map.find(type) != prec_map.end()) {
			prec = prec_map[type];
		}
		else {
			prec = 5;
		}
	}

	Expr::Expr(Expr &&a, expr_node &opt, Expr &&b)
		:id_type(std::move(a.id_type)),
		id_value(std::move(a.id_value)) {
		if(!a._sufexpr.empty() && !b._sufexpr.empty()) {
			_sufexpr = '(' + opt.name + ')' + a._sufexpr + b._sufexpr;
		}
		if(prec_map[a.root->type] < prec_map[opt.type]
			||
			(prec_map[a.root->type] == prec_map[opt.type]
			//不满足交换律
			&& (!exchangadble[a.root->type] || !exchangadble[opt.type])
			)) {
			a.node_pool.push_front(expr_node(LP, "("));
			a.node_pool.push_back(expr_node(RP, ")"));
		}
		if(prec_map[b.root->type] < prec_map[opt.type]
			||
			(prec_map[b.root->type] == prec_map[opt.type]
			//不满足交换律
			&& (!exchangadble[b.root->type] || !exchangadble[opt.type])
			)) {
			b.node_pool.push_front(expr_node(LP, "("));
			b.node_pool.push_back(expr_node(RP, ")"));
		}

		node_pool.splice(node_pool.end(), a.node_pool);
		node_pool.push_back(opt);
		root = &node_pool.back();
		root->subtree.push_back(a.root);
		root->subtree.push_back(b.root);
		node_pool.splice(node_pool.end(), b.node_pool);

		a.root = b.root = nullptr;
		for(auto &&i : std::move(b.id_type)) {
			id_type.insert(i);
		}
		for(auto &&i : std::move(b.id_value)) {
			id_value.insert(i);
		}
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
	stdexpr_map Expr::stdexpr(const expr_node *st, std::set<Expr*, Expr_ptr_cmp> &ptr_map) {
		auto check_map = [&](Expr *&ptr) {
			auto in_map = ptr_map.find(ptr);
			if(in_map != ptr_map.end()) {
				delete ptr;
				ptr = *in_map;
			}
			else {
				ptr_map.insert(ptr);
				ptr->init_sufexpr();
			}
		};
		if(!st) {
			auto ptr = new Expr("ERROR");
			check_map(ptr);
			return{ { { { ptr, 1 } }, 1 } };
		}
		switch(st->type) {
		case ERROR:{
			auto ptr = new Expr("ERROR");
			check_map(ptr);
			return{ { { { ptr, 1 } }, 1 } };
		}
		case ID:
		{
			auto ptr = new  Expr(st->name);
			check_map(ptr);
			return{ { { { ptr, 1 } }, 1 } };
		}
		case CONST:
		{
			auto ptr = new  Expr("1");
			check_map(ptr);
			return{ { { { ptr, 1 } }, to_FLOAT(st->name) } };
		}
		case ADD:
		{
			auto &&subexpr = stdexpr(st->subtree[0], ptr_map);
			subexpr += stdexpr(st->subtree[1], ptr_map);
			return std::move(subexpr);
		}
		case SUB:
		{
			auto &&expra = stdexpr(st->subtree[0], ptr_map);
			auto &&exprb = stdexpr(st->subtree[1], ptr_map);
			for(auto &i:exprb) {
				i.second = -i.second;
			}
			expra += exprb;
			return std::move(expra);
		}
		case MUL:
		{
			auto &&expra = stdexpr(st->subtree[0], ptr_map);
			auto &&exprb = stdexpr(st->subtree[1], ptr_map);
			if(
				exprb.size() == 1 && exprb.begin()->first.size() == 1
				&& exprb.begin()->first.begin()->first->root->type == CONST) {
				FLOAT c = to_FLOAT(exprb.begin()->first.begin()->first->root->name);
				for(auto &i : expra) {
					i.second *= c;
				}
			}
			else if(
				expra.size() == 1 && expra.begin()->first.size() == 1
				&& expra.begin()->first.begin()->first->root->type == CONST) {
				FLOAT c = to_FLOAT(expra.begin()->first.begin()->first->root->name);
				for(auto &i : exprb) {
					i.second *= c;
				}
				std::swap(expra, exprb);
			}
			else {
				expra *= exprb;
			}
			return std::move(expra);
		}
		case DIV:
		{
			auto &&expra = stdexpr(st->subtree[0], ptr_map);
			auto &&exprb = stdexpr(st->subtree[1], ptr_map);
			Expr *b = new Expr(to_expr(exprb));
			check_map(b);
			plyn_map b1 = { { b, 1 } };
			expra /= std::pair<plyn_map, FLOAT>{b1, 1};
			return std::move(expra);
		} 
		case MOD:
		{
			auto &&expra = stdexpr(st->subtree[0], ptr_map);
			auto &&exprb = stdexpr(st->subtree[1], ptr_map);
			auto ptr = new Expr(to_expr(expra) % to_expr(exprb));
			check_map(ptr);
			return{ { { { ptr, 1 } }, 1 } };
		}
		case POW:
		{
			auto &&expra = stdexpr(st->subtree[0], ptr_map);
			auto &&exprb = stdexpr(st->subtree[1], ptr_map);
			stdexpr_map ret;
			if(
				//(x1+x2...+xn)^m
				expra.size() > 1
				//exprb为整数
				&& exprb.size() == 1 && exprb.begin()->first.size() == 1
				&& exprb.begin()->first.begin()->first->root->type == CONST
				&& IS_INT(exprb.begin()->second)
				&& exprb.begin()->second > 0
				) {
				int n = (int)exprb.begin()->second;
				ret = expra;
				std::vector<stdexpr_map> factor;
				while(n) {
					if(n % 2) {
						factor.push_back(ret);
						if(n == 1) {
							break;
						}
					}
					n /= 2;
					ret *= ret;
				}
				ret = factor.back();
				factor.pop_back();
				for(auto &i : factor) {
					ret *= i;
				}
			}
			else {
				auto ptr = new Expr(to_expr(expra) ^ to_expr(exprb));
				check_map(ptr);
				plyn_map temp = { { ptr, 1 } };
				ret += {temp, 1};
			}
			return std::move(ret);
		}
		case NEGA:
		{
			auto &&expr = stdexpr(st->subtree[0], ptr_map);
			for(auto &i : expr) {
				i.second = -i.second;
			}
			return expr;
		}
		case POSI:
			return 	stdexpr(st->subtree[0], ptr_map);
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
			auto &&expr = stdexpr(st->subtree[0], ptr_map);
			Expr a(to_expr(expr));
			auto ptr = new Expr(call_func(a, st->type, st->name));
			check_map(ptr);
			return{ { {{ ptr, 1 } }, 1 } };
		}
		case LOG:
		case MAX:
		case MIN:
		case DIFF:
		{
			auto &&expra = stdexpr(st->subtree[0], ptr_map);
			auto &&exprb = stdexpr(st->subtree[1], ptr_map);

			Expr a(to_expr(expra));
			Expr b(to_expr(exprb));
			auto ptr = new Expr(call_func(a, b, st->type, st->name));
			check_map(ptr);
			return{ { { { ptr, 1 } }, 1 } };
		}
		default:
			error_message
				+= st->name + "is unkown symbol.\n";
			break;
		}
		auto ptr = new Expr("ERROR");
		check_map(ptr);
		return{ { { { ptr, 1 } }, 1 } };
	}
	bool Expr::operator<(Expr &b) {
		bool this_is_const = (root->type == CONST
			|| (operand[root->type] == 1 && root->subtree[0]->type == CONST));
		bool b_is_const = (b.root->type == CONST
			|| (operand[b.root->type] == 1 && b.root->subtree[0]->type == CONST));
		if(this_is_const && !b_is_const) {
			return true;
		}
		if(!this_is_const && b_is_const) {
			return false;
		}
		if(this_is_const && b_is_const) {
			auto &ca = root->type == CONST ? root->name : root->subtree[0]->name;
			auto &cb = b.root->type == CONST ? b.root->name : b.root->subtree[0]->name;
			if(ca.size() < cb.size()) {
				return true;
			}
			else if(ca.size() == cb.size()) {
				return ca < cb;
			}
			return false;
		}
		if(_sufexpr.empty()) {
			_sufexpr = sufexpr(root);
		}
		if(b._sufexpr.empty()) {
			b._sufexpr = sufexpr(b.root);
		}
		return _sufexpr < b._sufexpr;
	}
	bool Expr::operator<(const Expr &b) const {
		if(root->type == CONST && b.root->type != CONST) {
			return true;
		}
		if(root->type != CONST && b.root->type == CONST) {
			return false;
		}
		if(root->type == CONST && b.root->type == CONST) {
			if(root->name.size() < b.root->name.size()) {
				return true;
			}
			else if(root->name.size() == b.root->name.size()) {
				return root->name < b.root->name;
			}
			return false;
		}
		if(_sufexpr.empty()) {
			if(b._sufexpr.empty()) {
				return sufexpr(root) < sufexpr(b.root);
			}
			return sufexpr(root) < b._sufexpr;
		}
		else {
			if(b._sufexpr.empty()) {
				return _sufexpr < sufexpr(b.root);
			}
			return _sufexpr < b._sufexpr;
		}
	}
	bool Expr::operator<(const Expr &b) {
		if(root->type == CONST && b.root->type != CONST) {
			return true;
		}
		if(root->type != CONST && b.root->type == CONST) {
			return false;
		}
		if(root->type == CONST && b.root->type == CONST) {
			if(root->name.size() < b.root->name.size()) {
				return true;
			}
			else if(root->name.size() == b.root->name.size()) {
				return root->name < b.root->name;
			}
			return false;
		}
		if(_sufexpr.empty()) {
			_sufexpr = sufexpr(root);
		}
		if(b._sufexpr.empty()) {
			return _sufexpr < sufexpr(b.root);
		}
		else {
			return _sufexpr < b._sufexpr;
		}
	}
	bool Expr::operator<(Expr &b) const {
		if(root->type == CONST && b.root->type != CONST) {
			return true;
		}
		if(root->type != CONST && b.root->type == CONST) {
			return false;
		}
		if(root->type == CONST && b.root->type == CONST) {
			if(root->name.size() < b.root->name.size()) {
				return true;
			}
			else if(root->name.size() == b.root->name.size()) {
				return root->name < b.root->name;
			}
			return false;
		}
		if(b._sufexpr.empty()) {
			b._sufexpr = sufexpr(b.root);
		}
		if(_sufexpr.empty()) {
			return sufexpr(root) < b._sufexpr;
		}
		else {
			return _sufexpr < b._sufexpr;
		}
	}


	plyn_map& operator*=(plyn_map &a, const plyn_map &b) {
		for(auto i : b) {
			a[i.first] += i.second;
		}
		return a;
	}
	plyn_map& operator/=(plyn_map &a, const plyn_map &b) {
		for(auto i : b) {
			a[i.first] -= i.second;
		}
		return a;
	}
	stdexpr_map& operator+=(stdexpr_map &a, const std::pair<plyn_map, FLOAT> &b) {
		a[b.first] += b.second;
		return a;
	}
	stdexpr_map& operator*=(stdexpr_map &a, const std::pair<plyn_map, FLOAT> &b) {
		stdexpr_map temp;
		for(auto i : a) {
			temp[std::move(static_cast<plyn_map>(i.first) *= b.first)] += i.second * b.second;
		}
		std::swap(a, temp);
		return a;
	}
	stdexpr_map& operator/=(stdexpr_map &a, const std::pair<plyn_map, FLOAT> &b) {
		stdexpr_map temp;
		for(auto i : a) {
			temp[std::move(static_cast<plyn_map>(i.first) /= b.first)] += i.second / b.second;
		}
		std::swap(a, temp);
		return a;
	}
	stdexpr_map& operator+=(stdexpr_map &a, const stdexpr_map &b) {
		for(auto &i : b) {
			a[i.first] += i.second;
		}
		return a;
	}
	stdexpr_map& operator*=(stdexpr_map &a, const stdexpr_map &b) {
		stdexpr_map temp;
		for(const auto &i : b) {
			for(auto j : a) {
				temp[static_cast<plyn_map>(j.first) *= i.first] += i.second * j.second;
			}
		}
		std::swap(a, temp);
		return a;
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
				to_FLOAT(a.root->name) + to_FLOAT(b.root->name));
		}
		//-c1 + c2 = c3
		if(a.root->type == NEGA
			&& a.root->subtree[0]->type == CONST && b.root->type == CONST) {
			return
				to_string(
				to_FLOAT(b.root->name) - to_FLOAT(a.root->subtree[0]->name));
		}
		//c1 + -c2 = c3
		if(b.root->type == NEGA
			&& b.root->subtree[0]->type == CONST && a.root->type == CONST) {
			return
				to_string(
				to_FLOAT(a.root->name) - to_FLOAT(b.root->subtree[0]->name));
		}
		//-c1 + -c2 = c3
		if(a.root->type == NEGA && b.root->type == NEGA
			&& a.root->subtree[0]->type == CONST
			&& b.root->subtree[0]->type == CONST) {
			return
				Expr::nega(
				to_string(
				to_FLOAT(a.root->subtree[0]->name) + to_FLOAT(b.root->subtree[0]->name)));
		}
		if(a.root->type == MUL && b.root->type == MUL) {
			//n*x + m*x = (m+n) * x
			if(a.root->subtree[0]->type == CONST
				&& b.root->subtree[0]->type == CONST
				&& Expr::equal(a.root->subtree[1], b.root->subtree[1])) {
				return Expr(to_string(
					to_FLOAT(a.root->subtree[0]->name)
					+ to_FLOAT(b.root->subtree[0]->name))) * Expr::reverse_parse(b.root->subtree[1]);
			}
			//TODO
		}
		//n*x + x = (n+1)*x
		if(a.root->type == MUL) {
			// *
			//n x
			if(a.root->subtree[0]->type == CONST &&
				Expr::equal(a.root->subtree[1], b)) {
				return Expr(to_string(
					to_FLOAT(a.root->subtree[0]->name) + 1)) * std::move(b);
			}
			// *
			//x n
			if(a.root->subtree[1]->type == CONST &&
				Expr::equal(a.root->subtree[0], b)) {
				return Expr(to_string(
					to_FLOAT(a.root->subtree[1]->name) + 1))* std::move(b);
			}
		}
		//x + n*x = (n+1)*x
		if(b.root->type == MUL) {
			// *
			//n x
			if(b.root->subtree[0]->type == CONST &&
				Expr::equal(b.root->subtree[1], a)) {
				return Expr(to_string(
					to_FLOAT(b.root->subtree[0]->name) + 1))*(std::move(a));
			}
			// *
			//x n
			if(b.root->subtree[1]->type == CONST &&
				Expr::equal(b.root->subtree[0], a)) {
				return Expr(to_string(
					to_FLOAT(b.root->subtree[1]->name) + 1))*(std::move(a));
			}
		}
		//x + x = 2x
		if(Expr::equal(a, b)) {
			return Expr("2") * std::move(b);
		}
		//x + -x = 0
		if(b.root->type == NEGA && Expr::equal(a, b.root->subtree[0])) {
			return Expr("0");
		}
		if(a.root->type == NEGA && Expr::equal(a.root->subtree[0], b)) {
			return Expr("0");
		}
		//x-y + y = x
		if(b.root->type == SUB && Expr::equal(a, b.root->subtree[1])) {
			return Expr::reverse_parse(b.root->subtree[0]);
		}
		if(a.root->type == SUB && Expr::equal(b, a.root->subtree[1])) {
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
				to_FLOAT(a.root->name) - to_FLOAT(b.root->name));
		}
		//-c1 - c2 = c3
		if(a.root->type == NEGA
			&& a.root->subtree[0]->type == CONST && b.root->type == CONST) {
			return
				Expr::nega(
				to_string(
				to_FLOAT(b.root->name) + to_FLOAT(a.root->subtree[0]->name)));
		}
		//x - x = 0
		else if(Expr::equal(a, b)) {
			return Expr("0");
		}
		if(a.root->type == MUL && b.root->type == MUL) {
			//n*x - m*x = (m-n) * x
			if(a.root->subtree[0]->type == CONST
				&& b.root->subtree[0]->type == CONST
				&& Expr::equal(a.root->subtree[1], b.root->subtree[1])) {
				Expr(to_string(
					to_FLOAT(a.root->subtree[0]->name)
					- to_FLOAT(b.root->subtree[0]->name)))*Expr::reverse_parse(b.root->subtree[1]);
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
				to_FLOAT(a.root->name) * to_FLOAT(b.root->name));
		}
		//x * x = x^2
		if(Expr::equal(a, b)) {
			return Expr::opt(std::move(a), POW, "^", Expr("2"));
		}
		//m*x * n = m*n*x
		if(a.root->type == MUL && a.root->subtree[0]->type == CONST
			&& b.root->type == CONST) {
			a.root->subtree[0]->name 
				= to_string(to_FLOAT(a.root->subtree[0]->name)*to_FLOAT(b.root->name));
			return std::move(a);
		}
		//m * n*x = m*n*x
		if(b.root->type == MUL && b.root->subtree[0]->type == CONST
			&& a.root->type == CONST) {
			b.root->subtree[0]->name
				= to_string(to_FLOAT(b.root->subtree[0]->name)*to_FLOAT(a.root->name));
			return std::move(b);
		}
		// (fx / gx)*gx = fx
		if(b.root->type == DIV && Expr::equal(a, b.root->subtree[1])) {
			return Expr::reverse_parse(b.root->subtree[0]);
		}
		if(a.root->type == DIV && Expr::equal(b, a.root->subtree[1])) {
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
			&& Expr::equal(b.root->subtree[0], a)) {
			return Expr::reverse_parse(b.root->subtree[0])
				^ (Expr::reverse_parse(b.root->subtree[1]) + Expr("1"));
		}
		if(a.root->type == POW
			&& Expr::equal(b, a.root->subtree[0])) {
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
				to_FLOAT(a.root->name) / to_FLOAT(b.root->name));
		}
		//x / x = 1
		else if(Expr::equal(a, b)) {
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
			&& Expr::equal(a.root->subtree[0], b)) {
			return
				Expr("1") / Expr::reverse_parse(a.root->subtree[1]);
		}
		// x*y / x = y
		if(a.root->type == MUL) {
			if(Expr::equal(a.root->subtree[0], b)) {
				return Expr::reverse_parse(a.root->subtree[1]);
			}
			else if(Expr::equal(a.root->subtree[1], b)) {
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
				(int)to_FLOAT(b.root->name) : (int)to_FLOAT(b.root->subtree[0]->name);
			return n % 2 ? std::move(a) : Expr("1");
		}
		return Expr::opt(std::move(a), POW, "^", std::move(b));
	}
}