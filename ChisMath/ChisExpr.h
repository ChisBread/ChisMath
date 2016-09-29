#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <cmath>
#define M_PI 3.14159265358979323846
#define M_E 2.71828182845904523536
namespace chis {
	enum {
		ID, CONST, VARI,
		DOT,
		EQU,// =
		ADD, SUB, 
		MUL, DIV, MOD, POW,
		SIN, COS, TAN, COT,
		ARCSIN, ARCCOS, ARCTAN, ARCCOT,
		LOG, LN,
		LP, RP,
		DIFF,//Î¢·Ö
		MIN, MAX,
	};
	extern int max_typeid;
	extern std::map<std::string, int> keyword;
	extern std::map<int, int> prec_map;
	

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
			//½ûÖ¹¿½±´
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
			std::list<expr_node*> subtree;
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
		}
		Expr(const Expr &expr) {
			re_lexer lexer(node_pool, expr.node_pool);
			expr_parser parser(lexer);
			root = parser.parse();
		}
		Expr(const std::list<expr_node> &expr_node_pool) {
			re_lexer lexer(node_pool, expr_node_pool);
			expr_parser parser(lexer);
			root = parser.parse();
		}

		//½ûÖ¹¿½±´
		Expr& operator=(const Expr&) = delete;
		void init_id(int type) {
			for(auto &i : node_pool) {
				if(i.type == ID) {
					if(i.name == "pi") {
						id_type[i.name] = CONST;
						id_value[i.name] = M_PI;
					}
					else if(i.name == "e") {
						id_type[i.name] = CONST;
						id_value[i.name] = M_E;
					}
					else {
						id_type[i.name] = type;
						id_value[i.name] = 0;
					}
				}
			}
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
			return opt(*this, ADD, "+", b);
		}
		Expr operator-(const Expr &b) const {
			return opt(*this, SUB, "-", b);
		}
		Expr operator*(const Expr &b) const {
			return opt(*this, MUL, "*", b);
		}
		Expr operator/(const Expr &b) const {
			return opt(*this, DIV, "/", b);
		}
		Expr operator%(const Expr &b) const {
			return opt(*this, MOD, "%", b);
		}
		Expr operator^(const Expr &b) const {
			return opt(*this, POW, "^", b);
		}
		static Expr equal(const Expr &a, const Expr &b) {
			return opt(a, EQU, "=", b);
		}
		static Expr opt(const Expr &a, int type, const std::string &name, const Expr &b) {
			std::list<expr_node> nodes = a.node_pool;
			nodes.push_front(expr_node(LP, "("));
			nodes.push_back(expr_node(RP, ")"));
			nodes.push_back(expr_node(type, name));
			nodes.push_back(expr_node(LP, "("));
			for(auto &i : b.node_pool) {
				nodes.push_back(i);
			}
			nodes.push_back(expr_node(RP, ")"));
			return Expr(nodes);
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
			return call_func(b, LN, "ln");
		}
		static Expr log(const Expr &a, const Expr &b) {
			return call_func(a, b, LOG, "log");
		}
		static Expr max(const Expr &a, const Expr &b) {
			return call_func(a, b, MAX, "max");
		}
		static Expr min(const Expr &a, const Expr &b) {
			return call_func(a, b, MIN, "min");
		}
		static Expr call_func(const Expr &a, int type, const std::string &name) {
			std::list<expr_node> nodes;
			nodes.push_back(expr_node(type, name));
			nodes.push_back(expr_node(LP, "("));
			for(auto &i : a.node_pool) {
				nodes.push_back(i);
			}
			nodes.push_back(expr_node(RP, ")"));
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

	private:

		expr_node *root;
		std::map<std::string, int> id_type;
		std::map<std::string, double> id_value;
		std::list<expr_node> node_pool;
	};
	
}