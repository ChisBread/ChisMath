#pragma once
#include <iostream>
#include <fstream>
#include <strstream>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <forward_list>
#include <queue>
#include <map>
#include <set>
#include <cmath>
#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536
namespace chis {
	enum {
		ERROR,
		ID, CONST, VARI,
		DOT,
		EQU,// =
		NEGA, POSI, //-x +x
		ADD, SUB, 
		MUL, DIV, MOD, POW,
		SIN, COS, TAN, COT,
		ARCSIN, ARCCOS, ARCTAN, ARCCOT,
		LOG, LN,
		LP, RP,
		DIFF,//微分
		MIN, MAX,
	};

	extern int max_typeid;
	extern std::map<std::string, int> keyword;
	extern std::map<int, int> prec_map;
	extern std::map<int, bool> exchangadble;
	extern std::map<int, int> operand;
	double to_double(const std::string &num);
	std::string to_string(double num);
	class Expr;
	Expr operator+(Expr &&a, Expr &&b);
	Expr operator-(Expr &&a, Expr &&b);
	Expr operator*(Expr &&a, Expr &&b);
	Expr operator/(Expr &&a, Expr &&b);
	Expr operator%(Expr &&a, Expr &&b);
	Expr operator^(Expr &&a, Expr &&b);
	class Expr {
		friend Expr operator+(Expr &&a, Expr &&b);
		friend Expr operator-(Expr &&a, Expr &&b);
		friend Expr operator*(Expr &&a, Expr &&b);
		friend Expr operator/(Expr &&a, Expr &&b);
		friend Expr operator%(Expr &&a, Expr &&b);
		friend Expr operator^(Expr &&a, Expr &&b);
		class expr_parser;
		class expr_node {
			friend Expr operator+(Expr &&a, Expr &&b);
			friend Expr operator-(Expr &&a, Expr &&b);
			friend Expr operator*(Expr &&a, Expr &&b);
			friend Expr operator/(Expr &&a, Expr &&b);
			friend Expr operator%(Expr &&a, Expr &&b);
			friend Expr operator^(Expr &&a, Expr &&b);
			friend class Expr::expr_parser;
			friend class Expr;
		public:
			expr_node() {}
			expr_node(int type, std::string name) :type(type), name(name) {
				if(prec_map.find(type) != prec_map.end()) {
					prec = prec_map[type];
				}
				else {
					prec = 5;
				}
			}
			//禁止拷贝
			expr_node& operator=(const expr_node&) = delete;
			void insert_subtree(expr_node &st) {
				st.parent = this;
				subtree.push_back(&st);
			}
		private:
			int type;
			int prec;
			std::string name;
			expr_node* parent = nullptr;
			std::vector<expr_node*> subtree;
		};
		class _lexer {
		public:
			virtual const expr_node* lookahead() = 0;
			virtual const expr_node* lookback() = 0;
			virtual expr_node* get_token() = 0;
			virtual bool is_end() const = 0;
		};
		class expr_lexer:public _lexer {
		public:
			expr_lexer(std::list<expr_node> &node_pool, const std::string &expr) :token_pool(node_pool), expr(expr) {
				scan_index = expr.begin();
				end = expr.end();
			}
			virtual const expr_node* lookahead() override {
				if(token_buffer.empty()) {
					if(scan_index == end) {
						return &ERROR_NODE;
					}
					scan();
				}
				if(token_buffer.empty()) {
					return &ERROR_NODE;
				}
				return &token_buffer.front();
			}
			virtual expr_node* get_token() override {
				if(token_buffer.empty()) {
					if(scan_index == end) {
						return &ERROR_NODE;
					}
					scan();
				}
				if(token_buffer.empty()) {
					return &ERROR_NODE;
				}
				token_pool.push_back(token_buffer.front());
				token_buffer.pop();
				return &token_pool.back();
			}
			virtual bool is_end() const override {
				return scan_index == end && token_buffer.empty();
			}
			virtual const expr_node* lookback() override {
				if(token_pool.empty()) {
					return &ERROR_NODE;
				}
				return &token_pool.back();
			}
		private:
			void scan();
			std::queue<expr_node> token_buffer;
			std::list<expr_node> &token_pool;
			const std::string &expr;
			std::string::const_iterator scan_index;
			std::string::const_iterator end;
		};
		class re_lexer :public _lexer {
		public:
			re_lexer(std::list<expr_node> &node_pool, const std::list<expr_node> &expr) :
				scan_index(expr.begin()), end(expr.end()), token_pool(node_pool) {}
			virtual const expr_node* lookahead() override {
				const expr_node *node = nullptr;
				if(scan_index != end) {
					node = &(*scan_index);
				}
				else {
					return &ERROR_NODE;
				}
				return node;
			}
			virtual expr_node* get_token() override {
				expr_node *node = nullptr;
				if(scan_index != end) {
					token_pool.push_back(*scan_index);
					token_pool.back().subtree.clear();
					token_pool.back().parent = nullptr;
					++scan_index;
					node = &token_pool.back();
				}
				else {
					return &ERROR_NODE;
				}
				return node;
			}
			virtual bool is_end() const override {
				if(scan_index == end) {
					return true;
				}
				return false;
			}
			virtual const expr_node* lookback() override {
				if(token_pool.empty()) {
					return &ERROR_NODE;
				}
				return &token_pool.back();
			}
			std::list<expr_node> &token_pool;
			std::list<expr_node>::const_iterator scan_index;
			std::list<expr_node>::const_iterator end;
		};
		class expr_parser {
		public:
			expr_parser(_lexer &lexer) :lexer(lexer) {}
			expr_node* parse() {
				return parse_equ();
			}
		private:
			expr_node* parse_equ();
			expr_node* parse_add();
			expr_node* parse_mul();
			expr_node* parse_pow();
			expr_node* parse_unary();
			expr_node* parse_func();
			expr_node* parse_primary();
			_lexer &lexer;
		};
	public:
		struct Expr_ptr_cmp {
			bool operator()(const Expr *a, const Expr *b) const {
				return *a < *b;
			}
		};
		struct Expr_map_cmp {
			bool operator()(const std::map<Expr*, int, Expr_ptr_cmp> &a, const std::map<Expr*, int, Expr_ptr_cmp> &b) const {
				auto ia = a.begin(), ib = b.begin();
				for(; ia != a.end() && ib != b.end(); ++ia, ++ib) {
					if(*(ia->first) < *(ib->first)) {
						return true;
					}
					else if(*(ib->first) < *(ia->first)) {
						return false;
					}
					//first 相等
					else if(ia->second < ib->second) {
						return true;
					}
					else if(ib->second < ia->second) {
						return false;
					}
				}
				//abc < abcd
				return a.size() < b.size();
			}
		};
		using plyn_map = std::map<Expr*, int, Expr_ptr_cmp>;
		using stdexpr_map = std::map<plyn_map, int, Expr_map_cmp>;
	public:
		Expr(const std::string &expr) {
			expr_lexer lexer(node_pool, expr);
			expr_parser parser(lexer);
			root = parser.parse();
			if(!root) {
				node_pool.clear();
			}
			else {
				root->parent = &NIL;
				init_id();
			}
		}
		Expr(const Expr &expr) {
			node_pool = expr.node_pool;
			std::map<const expr_node*, expr_node*> tslt;
			auto j = expr.node_pool.begin();
			tslt[nullptr] = nullptr;
			tslt[&NIL] = &NIL;
			for(auto i = node_pool.begin(); i != node_pool.end(); ++i, ++j) {
				//将源节点地址翻译成待构造表达式的节点地址
				tslt[&(*j)] = &(*i);
			}
			for(auto &i : node_pool) {
				i.parent = tslt[i.parent];
				for(auto &j : i.subtree) {
					j = tslt[j];
				}
			}
			root = tslt[expr.root];
			id_type = expr.id_type;
			id_value = expr.id_value;
		}
		Expr(const std::list<expr_node> &expr_node_pool) {
			re_lexer lexer(node_pool, expr_node_pool);
			expr_parser parser(lexer);
			root = parser.parse();
			if(!root) {
				node_pool.clear();
			}
			else {
				root->parent = &NIL;
				init_id();
			}
		}
		Expr(const expr_node *rootb):Expr(reverse_parse(rootb)) {
			init_id();
		}
		Expr(Expr &&expr) 
			:id_type(std::move(expr.id_type)),
			id_value(std::move(expr.id_value)), _sufexpr(std::move(expr._sufexpr)) {
			node_pool.splice(node_pool.end(), expr.node_pool);
			root = expr.root;
			expr.root = nullptr;
		}
		Expr(Expr &&a, expr_node &opt, Expr &&b)
			:id_type(std::move(a.id_type)),
			id_value(std::move(a.id_value)) {
			if(prec_map[a.root->type] < prec_map[opt.type]
				||
				(prec_map[a.root->type] == prec_map[opt.type]
				//不满足交换律
				&& (!exchangadble[a.root->type] && !exchangadble[opt.type])
				)) {
				a.node_pool.push_front(expr_node(LP, "("));
				a.node_pool.push_back(expr_node(RP, ")"));
			}
			if(prec_map[b.root->type] < prec_map[opt.type]
				||
				(prec_map[b.root->type] == prec_map[opt.type]
				//不满足交换律
				&& (!exchangadble[b.root->type] && !exchangadble[opt.type])
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
		Expr(expr_node &opt, Expr &&a)
			:id_type(std::move(a.id_type)),
			id_value(std::move(a.id_value)) {
			node_pool.splice(node_pool.end(), a.node_pool);
			if(a.root->type != ID && a.root->type != CONST) {
				node_pool.push_front(expr_node(LP, "("));
				node_pool.push_back(expr_node(RP, ")"));
			}
			node_pool.push_front(opt);
			root = &node_pool.front();
			root->subtree.push_back(a.root);
			a.root = nullptr;
		}
		Expr(expr_node &opt, Expr &&a, Expr &&b)
			:id_type(std::move(a.id_type)),
			id_value(std::move(a.id_value)) {
			node_pool.splice(node_pool.end(), a.node_pool);
			node_pool.push_back(expr_node(DOT, ","));
			node_pool.splice(node_pool.end(), b.node_pool);
			node_pool.push_front(expr_node(LP, "("));
			node_pool.push_back(expr_node(RP, ")"));

			node_pool.push_front(opt);
			root = &node_pool.front();
			root->subtree.push_back(a.root);
			root->subtree.push_back(b.root);
			a.root = b.root = nullptr;
		}
		Expr& operator=(const Expr &expr) {
			operator=(Expr(expr));
			return *this;
		}
		Expr& operator=(Expr &&expr) {
			swap(expr);
			return *this;
		}
		void init_id() {
			for(auto &i : node_pool) {
				if(i.type == ID) {
					id_type[i.name] = ID;
					id_value[i.name] = 0;
				}
				//TODO CONST
			}
		}
		void init_sufexpr() {
			_sufexpr = sufexpr(root);
		}
		bool is_constexpr() const{
			//没有未知数
			if(id_type.empty()) {
				return true;
			}
			return false;
		}
		const expr_node* get_root() const {
			return root;
		}
		std::string string_expr() const{
			std::string ret;
			for(auto i : node_pool) {
				ret += i.name;
			}
			return ret;
		}
		void set_id_type(const std::vector<std::pair<std::string, int>> &types) {
			for(auto &t : types) {
				id_type[t.first] = t.second;
			}
		}
		void set_id_value(const std::vector<std::pair<std::string, double>> &values) {
			for(auto &v : values) {
				id_value[v.first] = v.second;
			}
		}
		
		bool operator==(const Expr &b) const {
			return equal(this->root, b.root);
		}
		static Expr make_equal(const Expr &a, const Expr &b) {
			return opt(a, EQU, "=", b);
		}
		static Expr opt(const Expr &a, int type, const std::string &name, const Expr &b) {
			return Expr(Expr(a), expr_node(type, name), Expr(b));
		}
		static Expr opt(const Expr &a, int type, const std::string &name, Expr &&b) {
			return Expr(Expr(a), expr_node(type, name), std::move(b));
		}
		static Expr opt(Expr &&a, int type, const std::string &name, const Expr &b) {
			return Expr(std::move(a), expr_node(type, name), Expr(b));
		}
		static Expr opt(Expr &&a, int type, const std::string &name, Expr &&b) {
			return Expr(std::move(a), expr_node(type, name), std::move(b));
		}
		static Expr nega(Expr &&b) {
			if(b.root->type == NEGA) {
				return reverse_parse(b.root->subtree[0]);
			}
			if(b.root->name == "0") {
				return b;
			}
			return call_func(b, NEGA, "-");
		}
		static Expr posi(Expr &&b) {
			return call_func(b, POSI, "+");
		}
		static Expr sin(Expr &&b) {
			return call_func(b, SIN, "sin");
		}
		static Expr cos(Expr &&b) {
			return call_func(b, COS, "cos");
		}
		static Expr tan(Expr &&b) {
			return call_func(b, TAN, "tan");
		}
		static Expr cot(Expr &&b) {
			return call_func(b, COT, "cot");
		}
		static Expr arcsin(Expr &&b) {
			return call_func(b, ARCSIN, "arcsin");
		}
		static Expr arccos(Expr &&b) {
			return call_func(b, ARCCOS, "arccos");
		}
		static Expr arctan(Expr &&b) {
			return call_func(b, ARCTAN, "arctan");
		}
		static Expr arccot(Expr &&b) {
			return call_func(b, ARCCOT, "arccot");
		}
		static Expr ln(Expr &&b) {
			if(b.root->name == "e") {
				return Expr("1");
			}
			if(b.root->type == POW && b.root->subtree[0]->name == "e") {
				return reverse_parse(b.root->subtree[1]);
			}
			return call_func(b, LN, "ln");
		}
		static Expr log(Expr &&a, Expr &&b) {
			if(equal(a.root, b.root)) {
				return Expr("1");
			}

			if(a.root->type == POW) {
				//log(gx^a, fx^b) = (b/a)*log(gx,fx)
				if(b.root->type == POW) {
					return
						(reverse_parse(b.root->subtree[1]) / reverse_parse(a.root->subtree[1]))
						* log(reverse_parse(a.root->subtree[0]), reverse_parse(b.root->subtree[0]));
				}
				//log(gx^a, fx) = log(gx,fx)/a
				else {
					return  log(reverse_parse(a.root->subtree[0]), std::move(b)) / reverse_parse(a.root->subtree[1]);
				}
			}
			if(b.root->type == POW) {
				//log(gx^a, fx^b) = (b/a)*log(gx,fx)
				if(a.root->type == POW) {
					return
						(reverse_parse(b.root->subtree[1]) / reverse_parse(a.root->subtree[1]))
						* log(reverse_parse(a.root->subtree[0]), reverse_parse(b.root->subtree[0]));
				}
				//log(gx, fx^b) = (b)*log(gx,fx)
				else {
					return reverse_parse(b.root->subtree[1]) * log(std::move(a), reverse_parse(b.root->subtree[0]));
				}
			}
			
			return call_func(a, b, LOG, "log");
		}
		static Expr max(Expr &&a, Expr &&b) {
			if(a.root->type == CONST && b.root->type == CONST) {
				double da = to_double(a.root->name), db = to_double(b.root->name);
				return to_string(da > db ? da : db);
			}
			return call_func(a, b, MAX, "max");
		}
		static Expr min(Expr &&a, Expr &&b) {
			if(a.root->type == CONST && b.root->type == CONST) {
				double da = to_double(a.root->name), db = to_double(b.root->name);
				return to_string(da < db ? da : db);
			}
			return call_func(a, b, MIN, "min");
		}
		static Expr call_func(const Expr &a, int type, const std::string &name) {
			return Expr(expr_node(type, name), Expr(a));
		}
		static Expr call_func(Expr &&a, int type, const std::string &name) {
			return Expr(expr_node(type, name), std::move(a));
		}

		static Expr call_func(const Expr &a, const Expr &b, int type, const std::string &name) {
			return Expr(expr_node(type, name),Expr(a), Expr(b));
		}
		static Expr call_func(Expr &&a, const Expr &b, int type, const std::string &name) {
			return Expr(expr_node(type, name), std::move(a), Expr(b));
		}
		static Expr call_func(const Expr &a, Expr &&b, int type, const std::string &name) {
			return Expr(expr_node(type, name), Expr(a), std::move(b));
		}
		static Expr call_func(Expr &&a, Expr &&b, int type, const std::string &name) {
			return Expr(expr_node(type, name), std::move(a), std::move(b));
		}

		static Expr make_diff(const Expr &fx, const Expr &gx, const std::string &x) {
			return make_diff(fx, x) / make_diff(gx, x);
		}
		static Expr make_diff(const Expr &y, const std::string &x) {
			return diff(y.root, x);
		}
		static bool same_plynomials(const Expr &a, const Expr &b) {
			//同属一个整式
			if(a.root->type == CONST && b.root->type == CONST) {
				return true;
			}
			if(a.root->type == ID && b.root->type == ID) {
				return a.root->name == b.root->name;
			}
			if(a.root->type == CONST && b.root->type == MUL) {
				if(b.root->subtree[0]->type == CONST) {
					return equal(a.root, b.root->subtree[1]);
				}
				if(b.root->subtree[1]->type == CONST) {
					return equal(a.root, b.root->subtree[0]);
				}
			}
			if(b.root->type == CONST && a.root->type == MUL) {
				if(a.root->subtree[0]->type == CONST) {
					return equal(b.root, a.root->subtree[1]);
				}
				if(a.root->subtree[1]->type == CONST) {
					return equal(b.root, a.root->subtree[0]);
				}
			}
			if(a.root->type == MUL && b.root->type == MUL) {
				if(
					a.root->subtree[0]->type == CONST && 
					b.root->subtree[0]->type == CONST) {
					return equal(b.root->subtree[1], a.root->subtree[1]);
				}
			}
			return equal(a, b);
		}
		static Expr to_expr(stdexpr_map &exprs) {
			std::list<Expr> addexp;
			for(auto &i : exprs) {
				std::list<Expr> unequ;
				for(auto &j:i.first) {
					//a1^n1*a2^n2....
					if(j.first->root->type == CONST) {
						unequ.push_back(Expr(to_string(
							std::pow(to_double(j.first->root->name), j.second))));
					}
					else {
						unequ.push_back(Expr(*j.first) ^ Expr(to_string(j.second)));
					}
				}
				addexp.push_back(Expr("1"));
				for(auto &j : unequ) {
					addexp.back() = std::move(addexp.back()) * std::move(j);
				}
				addexp.back() = Expr(to_string(i.second) * std::move(addexp.back()));
				addexp.back()._sufexpr.clear();
			}
			Expr ret("0");
			for(auto &i:addexp) {
				ret = std::move(ret)+std::move(i);
			}
			return ret;
		}
		bool operator<(Expr &b) {
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
		bool operator<(const Expr &b) const{
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
		bool operator<(const Expr &b) {
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
		bool operator<(Expr &b) const {
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
		void swap(Expr &b) {
			std::swap(root, b.root);
			std::swap(id_type, b.id_type);
			std::swap(id_value, b.id_value);
			std::swap(node_pool, b.node_pool);
		}
		Expr& reverse_parse() {
			swap(reverse_parse(root));
			return *this;
		}
		const std::string& errors() const {
			return error_message;
		}
		void clear_errors() const {
			error_message.clear();
		}
		Expr stdexpr() const{
			std::set<Expr*, Expr_ptr_cmp> ptr_map;
			auto &&expr_list = stdexpr(root, ptr_map);
			auto &&expr = to_expr(expr_list);
			for(auto i : ptr_map) {
				delete i;
			}
			return std::move(expr);
		}
		void print() {
			print(root);
		}
		static void print(const expr_node *root) {
			std::list<const expr_node*> next_ply(1, root);
			auto empty = [&]() {
				for(auto &i : next_ply) {
					if(i) {
						return false;
					}
				}
				return true;
			};
			int high = ast_ply(root)*2;
			std::vector<std::list<const expr_node*>> pr;
			while(!empty()) {
				std::cout << std::string(high, ' ');
				----high;
				for(auto &i:next_ply) {
					if(i) {
						std::cout << i->name << " ";
					}
					else {
						std::cout << "  ";
					}
				}
				std::cout << std::endl;
				std::list<const expr_node*> temp;
				for(auto &i : next_ply) {
					int s = 2;
					if(i) {
						for(auto j : i->subtree) {
							--s;
							temp.push_back(j);
						}
					}
					for(int j = 0; j < s; ++j) {
						temp.push_back(nullptr);
					}
				}
				next_ply = temp;
			}
		}
		static int ast_ply(const expr_node *root) {
			if(!root) {
				return 0;
			}
			int l = root->subtree.empty() ? 0 : ast_ply(root->subtree[0]);
			int r = root->subtree.size() < 2 ? 0 : ast_ply(root->subtree[1]);
			return (l > r ? l : r) + 1;
		}
	private:
		static Expr reverse_parse(const expr_node *subroot);
		static bool equal(const expr_node *a, const expr_node *b) {
			return sufexpr(a)==sufexpr(b);
		}
		static std::string sufexpr(const expr_node *r) {
			std::string expr = r->name;
			if(!(r->subtree.empty())) {
				for(auto &i:r->subtree) {
					expr += sufexpr(i);
				}
			}
			return std::move(expr)+"|";
		}
		static bool equal(const Expr &a, const Expr &b) {
			if(a._sufexpr.empty()) {
				if(b._sufexpr.empty()) {
					return sufexpr(a.root) == sufexpr(b.root);
				}
				return sufexpr(a.root) == b._sufexpr;
			}
			else {
				if(b._sufexpr.empty()) {
					return a._sufexpr == sufexpr(b.root);
				}
				return a._sufexpr == b._sufexpr;
			}
		}
		static bool equal(Expr &a, Expr &b) {
			if(a._sufexpr.empty()) {
				a._sufexpr = sufexpr(a.root);
			}
			if(b._sufexpr.empty()) {
				b._sufexpr = sufexpr(b.root);
			}
			return a._sufexpr == b._sufexpr;
		}
		static bool equal(const Expr &a, Expr &b) {
			if(b._sufexpr.empty()) {
				b._sufexpr = sufexpr(b.root);
			}
			if(a._sufexpr.empty()) {
				return sufexpr(a.root) == b._sufexpr;
			}
			else {
				return a._sufexpr == b._sufexpr;
			}
		}
		static bool equal(Expr &a, const Expr &b) {
			if(a._sufexpr.empty()) {
				a._sufexpr = sufexpr(a.root);
			}
			if(b._sufexpr.empty()) {
				return sufexpr(b.root) == a._sufexpr;
			}
			else {
				return b._sufexpr == a._sufexpr;
			}
		}
		static bool equal(expr_node *a, Expr &b) {
			if(b._sufexpr.empty()) {
				b._sufexpr = sufexpr(b.root);
			}
			return sufexpr(a) == b._sufexpr;
		}
		static bool equal(Expr &a, expr_node *b) {
			if(a._sufexpr.empty()) {
				a._sufexpr = sufexpr(a.root);
			}
			return sufexpr(b) == a._sufexpr;
		}
		static stdexpr_map stdexpr(const expr_node *subroot, std::set<Expr*, Expr_ptr_cmp> &ptr_map);
		static Expr diff(const expr_node *subroot, const std::string &x);
		static std::string error_message;
		static expr_node ERROR_NODE;
		static expr_node NIL;
		expr_node *root;
		std::map<std::string, int> id_type;
		std::map<std::string, double> id_value;
		std::list<expr_node> node_pool;
		std::string _sufexpr;
	};
	using plyn_map = Expr::plyn_map;
	using stdexpr_map = Expr::stdexpr_map;
	plyn_map& operator*=(plyn_map &a, const plyn_map &b);
	stdexpr_map& operator+=(stdexpr_map &a, const std::pair<plyn_map, int> &b);
	stdexpr_map& operator*=(stdexpr_map &a, const std::pair<plyn_map, int> &b);
	stdexpr_map& operator+=(stdexpr_map &a, const stdexpr_map &b);
	stdexpr_map& operator*=(stdexpr_map &a, const stdexpr_map &b);

}