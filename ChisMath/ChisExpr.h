#pragma once
#include <fstream>
#include <strstream>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <set>
#include <cmath>
#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536
namespace chis {
	enum {
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
	
	double to_double(const std::string &num);
	std::string to_string(double num);

	class Expr {
		class expr_parser;
		class expr_node {
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
						//throw("wtf");
						return nullptr;
					}
					scan();
				}
				return &token_buffer.front();
			}
			virtual expr_node* get_token() override {
				if(token_buffer.empty()) {
					if(scan_index == end) {
						//throw("wtf");
						return nullptr;
					}
					scan();
				}
				token_pool.push_back(token_buffer.front());
				token_buffer.pop();
				return &token_pool.back();
			}
			virtual bool is_end() const override {
				return scan_index == end && token_buffer.empty();
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
				return node;
			}
			virtual bool is_end() const override {
				if(scan_index == end) {
					return true;
				}
				return false;
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
		Expr(const std::string &expr) {
			expr_lexer lexer(node_pool, expr);
			expr_parser parser(lexer);
			root = parser.parse();
			init_id();
		}
		Expr(const Expr &expr) {
			re_lexer lexer(node_pool, expr.node_pool);
			expr_parser parser(lexer);
			root = parser.parse();
			init_id();
		}
		Expr(const std::list<expr_node> &expr_node_pool) {
			re_lexer lexer(node_pool, expr_node_pool);
			expr_parser parser(lexer);
			root = parser.parse();
			init_id();
		}
		Expr(const expr_node *root):Expr(reverse_parse(root)) {
			init_id();
		}
		//禁止拷贝
		Expr& operator=(const Expr&) = delete;
		Expr& operator=(Expr&& expr) {
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
		bool is_constexpr() const{
			//没有未知数
			if(id_type.empty()) {
				return true;
			}
			return false;
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
		Expr operator+(const Expr &b) const {

			// x + 0 = x
			if(this->root->name == "0") {
				return b;
			}
			else if(b.root->name == "0") {
				return *this;
			}
			//c1 + c2 = c3
			if(this->root->type == CONST && b.root->type == CONST) {
				return to_string(
					to_double(this->root->name) + to_double(b.root->name));
			}
			//x + x = 2x
			else if(equal(this->root, b.root)) {
				return opt(Expr("2"), MUL, "*", b);
			}
			// x + -y = x - y
			if(b.root->type == NEGA) {
				return *this - reverse_parse(b.root->subtree[0]);
			}
			//x-y + y = x
			if(b.root->type == SUB && equal(this->root, b.root->subtree[1])) {
				return reverse_parse(b.root->subtree[0]);
			}
			if(this->root->type == SUB && equal(b.root, this->root->subtree[1])) {
				return reverse_parse(this->root->subtree[0]);
			}
			if(this->root->type == MUL && b.root->type == MUL) {
				//a1*a2 + b1*b2
				expr_node *a1 = this->root->subtree[0];
				expr_node *a2 = this->root->subtree[1];
				expr_node *b1 = b.root->subtree[0];
				expr_node *b2 = b.root->subtree[1];

				if(equal(a1, b1)) {
					return (reverse_parse(a2) + reverse_parse(b2))*reverse_parse(a1);
				}
				else if(equal(a1, b2)) {
					return (reverse_parse(a2) + reverse_parse(b1))*reverse_parse(a1);
				}
				else if(equal(a2, b2)) {
					return (reverse_parse(a1) + reverse_parse(b1))*reverse_parse(a2);
				}
				else if(equal(a2, b1)) {
					return (reverse_parse(a1) + reverse_parse(b2))*reverse_parse(a2);
				}
			}
			return opt(*this, ADD, "+", b);
		}
		Expr operator-(const Expr &b) const {
			// 0 - x = -x
			// x - 0 = x
			if(this->root->name == "0") {
				return nega(b);
			}
			else if(b.root->name == "0") {
				return *this;
			}
			//c1 - c2 = c3
			if(this->root->type == CONST && b.root->type == CONST) {
				return to_string(
					to_double(this->root->name) - to_double(b.root->name));
			}
			//x - x = 0
			else if(equal(this->root, b.root)) {
				return Expr("0");
			}
			// x-y - z = x-(y+z)
			if(this->root->type == SUB) {
				return
					reverse_parse(this->root->subtree[0])
					- (reverse_parse(this->root->subtree[1]) + b);
			}
			if(this->root->type == MUL && b.root->type == MUL) {
				//a1*a2 + b1*b2
				expr_node *a1 = this->root->subtree[0];
				expr_node *a2 = this->root->subtree[1];
				expr_node *b1 = b.root->subtree[0];
				expr_node *b2 = b.root->subtree[1];

				if(equal(a1, b1)) {
					return (reverse_parse(a2) - reverse_parse(b2))*reverse_parse(a1);
				}
				else if(equal(a1, b2)) {
					return (reverse_parse(a2) - reverse_parse(b1))*reverse_parse(a1);
				}
				else if(equal(a2, b2)) {
					return (reverse_parse(a1) - reverse_parse(b1))*reverse_parse(a2);
				}
				else if(equal(a2, b1)) {
					return (reverse_parse(a1) - reverse_parse(b2))*reverse_parse(a2);
				}
			}
			return opt(*this, SUB, "-", b);
		}
		Expr operator*(const Expr &b) const {
			// x * 1 = x
			if(this->root->name == "1") {
				return b;
			}
			else if(b.root->name == "1") {
				return *this;
			}
			if(this->root->name == "0" || b.root->name == "0") {
				return Expr("0");
			}
			//c1 * c2 = c3
			if(this->root->type == CONST && b.root->type == CONST) {
				return to_string(
					to_double(this->root->name) * to_double(b.root->name));
			}
			//x * x = x^2
			else if(equal(this->root, b.root)) {
				return opt(*this, POW, "^", Expr("2"));
			}
			// (fx / gx)*gx = fx
			if(b.root->type == DIV && equal(this->root, b.root->subtree[1])) {
				return reverse_parse(b.root->subtree[0]);
			}
			if(this->root->type == DIV && equal(b.root, this->root->subtree[1])) {
				return reverse_parse(this->root->subtree[0]);
			}
			// gx^a * gx^b = gx^(a+b)
			if(b.root->type == POW && this->root->type == POW
				&& equal(b.root->subtree[0], this->root->subtree[0])) {
				return reverse_parse(b.root->subtree[0]) 
					^ (reverse_parse(this->root->subtree[1]) + reverse_parse(b.root->subtree[1]));
			}
			return opt(*this, MUL, "*", b);
		}
		Expr operator/(const Expr &b) const {
			// 0 / x = 0
			if(this->root->name == "0") {
				return Expr("0");
			}
			// x / 1 = x
			if(b.root->name == "1") {
				return *this;
			}
			//TODO c1 / c2 = c3
			if(this->root->type == CONST && b.root->type == CONST) {
				return 
					to_string(
					to_double(this->root->name) / to_double(b.root->name));
			}
			//x / x = 1
			else if(equal(this->root, b.root)) {
				return Expr("1");
			}
			// gx^a / gx^b = gx^(a-b)
			if(b.root->type == POW && this->root->type == POW
				&& equal(b.root->subtree[0], this->root->subtree[0])) {
				return 
					reverse_parse(b.root->subtree[0])
					^ (reverse_parse(this->root->subtree[1]) - reverse_parse(b.root->subtree[1]));
			}
			// x/y / z = x/(y*z)
			if(this->root->type == DIV) {
				return
					reverse_parse(this->root->subtree[0]) 
					/ (reverse_parse(this->root->subtree[1]) * b);
			}
			// x/y / x = 1/y
			if(this->root->type == DIV 
				&& equal(this->root->subtree[0], b.root)) {
				return
					Expr("1") / reverse_parse(this->root->subtree[1]);
			}
			// x*y / x = y
			if(this->root->type == MUL) {
				if(equal(this->root->subtree[0], b.root)) {
					return reverse_parse(this->root->subtree[1]);
				}
				else if(equal(this->root->subtree[1], b.root)) {
					return reverse_parse(this->root->subtree[0]);
				}
			}

			return opt(*this, DIV, "/", b);
		}
		Expr operator%(const Expr &b) const {
			return opt(*this, MOD, "%", b);
		}
		Expr operator^(const Expr &b) const {
			// x ^ 1 = x
			if(b.root->name == "1") {
				return *this;
			}
			// x ^ 0 = 1
			if(b.root->name == "0") {
				return Expr("1");
			}
			// (x ^ g) ^ f -> x ^(g*f)
			if(this->root->type == POW) {
				return
					reverse_parse(this->root->subtree[0])
					^ (reverse_parse(this->root->subtree[1]) * b);
			}

			return opt(*this, POW, "^", b);
		}
		bool operator==(const Expr &b) const {
			return equal(this->root, b.root);
		}
		static Expr equal(const Expr &a, const Expr &b) {
			return opt(a, EQU, "=", b);
		}
		static Expr opt(const Expr &a, int type, const std::string &name, const Expr &b) {
			std::list<expr_node> nodes = a.node_pool;
			if(prec_map[a.root->type] < prec_map[type] 
				|| 
				(prec_map[a.root->type] == prec_map[type]
				//不满足交换律
				&& (a.root->type == DIV || a.root->type == SUB))) {
				nodes.push_front(expr_node(LP, "(")); 
				nodes.push_back(expr_node(RP, ")")); 
			}
			nodes.push_back(expr_node(type, name));
			if(prec_map[b.root->type] < prec_map[type]
				|| 
				(prec_map[b.root->type] == prec_map[type] 
				//不满足交换律
				&& (type == DIV || type == SUB)))
				nodes.push_back(expr_node(LP, "("));
			for(auto &i : b.node_pool) {
				nodes.push_back(i);
			}
			if(prec_map[b.root->type] < prec_map[type]
				|| 
				(prec_map[b.root->type] == prec_map[type] 
				//不满足交换律
				&& (type == DIV || type == SUB)))
				nodes.push_back(expr_node(RP, ")"));
			return Expr(nodes);
		}
		static Expr nega(const Expr &b) {
			return call_func(b, SUB, "-");
		}
		static Expr posi(const Expr &b) {
			return call_func(b, ADD, "+");
		}
		static Expr sin(const Expr &b) {
			return call_func(b, SIN, "sin");
		}
		static Expr cos(const Expr &b) {
			return call_func(b, COS, "cos");
		}
		static Expr tan(const Expr &b) {
			return call_func(b, TAN, "tan");
		}
		static Expr cot(const Expr &b) {
			return call_func(b, COT, "cot");
		}
		static Expr arcsin(const Expr &b) {
			return call_func(b, ARCSIN, "arcsin");
		}
		static Expr arccos(const Expr &b) {
			return call_func(b, ARCCOS, "arccos");
		}
		static Expr arctan(const Expr &b) {
			return call_func(b, ARCTAN, "arctan");
		}
		static Expr arccot(const Expr &b) {
			return call_func(b, ARCCOT, "arccot");
		}
		static Expr ln(const Expr &b) {
			if(b.root->name == "e") {
				return Expr("1");
			}
			return call_func(b, LN, "ln");
		}
		static Expr log(const Expr &a, const Expr &b) {
			if(equal(a.root, b.root)) {
				return Expr("1");
			}
			return call_func(a, b, LOG, "log");
		}
		static Expr max(const Expr &a, const Expr &b) {
			if(a.root->type == CONST && b.root->type == CONST) {
				double da = to_double(a.root->name), db = to_double(b.root->name);
				return to_string(da > db ? da : db);
			}
			return call_func(a, b, MAX, "max");
		}
		static Expr min(const Expr &a, const Expr &b) {
			if(a.root->type == CONST && b.root->type == CONST) {
				double da = to_double(a.root->name), db = to_double(b.root->name);
				return to_string(da < db ? da : db);
			}
			return call_func(a, b, MIN, "min");
		}
		static Expr call_func(const Expr &a, int type, const std::string &name) {
			std::list<expr_node> nodes;
			nodes.push_back(expr_node(type, name));
			if(a.root->type != ID && a.root->type != CONST) nodes.push_back(expr_node(LP, "("));
			for(auto &i : a.node_pool) {
				nodes.push_back(i);
			}
			if(a.root->type != ID && a.root->type != CONST) nodes.push_back(expr_node(RP, ")"));
			return Expr(nodes);
		}
		static Expr call_func(const Expr &a, const Expr &b, int type, const std::string &name) {
			std::list<expr_node> nodes = a.node_pool;
			nodes.push_front(expr_node(LP, "("));
			nodes.push_front(expr_node(type, name));
			nodes.push_back(expr_node(DOT, ","));
			for(auto &i : b.node_pool) {
				nodes.push_back(i);
			}
			nodes.push_back(expr_node(RP, ")"));
			return Expr(nodes);
		}
		static Expr make_diff(const Expr &y, const std::string &x) {
			return diff(y.root, x);
		}
		static Expr reverse_parse(const expr_node *subroot);
		bool operator<(const Expr &b) const {
			return less_than(root, b.root);
		}
		void swap(Expr &b) {
			std::swap(root, b.root);
			std::swap(id_type, b.id_type);
			std::swap(id_value, b.id_value);
			std::swap(node_pool, b.node_pool);
		}

		void standardization() {
			swap(standardization(root));
		}
	private:
		static bool less_than(const expr_node *a, const expr_node *b) {
			Expr &&ea = reverse_parse(a), &&eb = reverse_parse(b);
			if(a->type == CONST && b->type != CONST) {
				return true;
			}
			if(a->type != CONST && b->type == CONST) {
				return false;
			}
			if(ea.node_pool.size() < eb.node_pool.size()) {
				return true;
			}
			else if(ea.node_pool.size() == eb.node_pool.size()) {
				auto ba = ea.node_pool.begin();
				auto bb = eb.node_pool.begin();
				for(; ba != ea.node_pool.end(); ++ba, ++bb) {
					if(ba->name.size() < bb->name.size()) {
						return true;
					}
					else if(ba->name.size() == bb->name.size()) {
						if(ba->name < bb->name) {
							return true;
						}
						if(ba->name > bb->name) {
							break;
						}
					}
					else {
						break;
					}
				}
			}
			return false;
		}

		static bool equal(const expr_node *a, const expr_node *b) {
			Expr &&stda = standardization(a);
			Expr &&stdb = standardization(b);
			if(stda.node_pool.size() == stdb.node_pool.size()) {
				auto ba = stda.node_pool.begin();
				auto bb = stdb.node_pool.begin();
				for(; ba != stda.node_pool.end(); ++ba, ++bb) {
					if(ba->name != bb->name) {
						return false;
					}
				}
				return true;
			}
			return false;
		}
		static Expr standardization(const expr_node *subroot);
		static Expr diff(const expr_node *subroot, const std::string &x);
		
		expr_node *root;
		std::map<std::string, int> id_type;
		std::map<std::string, double> id_value;
		std::list<expr_node> node_pool;
	};
	
}