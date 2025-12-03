// casio_pp.cpp
// Compile: g++ -std=c++17 casio_pp.cpp -o casio_pp

#include <bits/stdc++.h>
#include <complex>
using namespace std;

using Complex = complex<double>;

// ----------------- Utilities -----------------
static double PI = acos(-1.0);
string trim(const string &s){
    size_t a = s.find_first_not_of(" \t\r\n");
    if(a==string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}
bool isNumberChar(char c){ return isdigit(c) || c=='.'; }
bool isAlpha(char c){ return isalpha((unsigned char)c) || c=='_'; }

// ----------------- Tokenizer -----------------
enum TokenType {T_NUM, T_OP, T_LP, T_RP, T_FUNC, T_COMMA, T_VAR};

struct Token {
    TokenType type;
    string text;
    Token(TokenType t= T_OP, string s=""): type(t), text(s) {}
};

// Tokenize: supports numbers, complex 'i' suffix, identifiers, operators, parentheses, comma
vector<Token> tokenize(const string &expr){
    vector<Token> out;
    size_t i=0, n=expr.size();
    while(i<n){
        char c = expr[i];
        if(isspace((unsigned char)c)){ i++; continue; }
        if(c=='('){ out.emplace_back(T_LP,"("); i++; continue;}
        if(c==')'){ out.emplace_back(T_RP,")"); i++; continue;}
        if(c==','){ out.emplace_back(T_COMMA,","); i++; continue;}
        // operators (including ^)
        if(string("+-*/^").find(c)!=string::npos){
            string s(1,c);
            // handle unary +/-
            if((c=='+'||c=='-')){
                if(out.empty() || out.back().type==T_OP || out.back().type==T_LP || out.back().type==T_COMMA){
                    // unary treat as part of number if next is digit or '.' or 'i'
                    size_t j=i+1;
                    if(j<n && (isNumberChar(expr[j]) || expr[j]=='i')){
                        // parse number
                        string num(1,c);
                        j=i+1;
                        while(j<n && (isNumberChar(expr[j])||expr[j]=='e'||expr[j]=='E' || expr[j]=='+'||expr[j]=='-')){ // handle e
                            num.push_back(expr[j]); j++;
                        }
                        // check for complex 'i'
                        bool complex_i = false;
                        if(j<n && expr[j]=='i'){ complex_i=true; num.push_back('i'); j++; }
                        out.emplace_back(T_NUM, num);
                        i=j;
                        continue;
                    }
                }
            }
            out.emplace_back(T_OP,s);
            i++; continue;
        }
        // number
        if(isNumberChar(c)){
            string num;
            while(i<n && (isNumberChar(expr[i]) || expr[i]=='e' || expr[i]=='E' || expr[i]=='+' || expr[i]=='-')){ // simple e handling (may overconsume + - in exponent but okay)
                // break on encountering alpha that is not e/E
                if((expr[i]=='+'||expr[i]=='-') && !num.empty() && (num.back()=='e' || num.back()=='E')){ num.push_back(expr[i]); i++; continue;}
                if(expr[i]=='+'||expr[i]=='-'){
                    if(!num.empty() && (num.back()=='e' || num.back()=='E')) { num.push_back(expr[i]); i++; continue;}
                }
                if(isNumberChar(expr[i]) || expr[i]=='e' || expr[i]=='E' ) { num.push_back(expr[i]); i++; }
                else break;
            }
            // check i
            if(i<n && expr[i]=='i'){ num.push_back('i'); i++; }
            out.emplace_back(T_NUM, num);
            continue;
        }
        // complex imaginary unit alone 'i'
        if(c=='i'){
            out.emplace_back(T_NUM, string("1i"));
            i++; continue;
        }
        // identifier: function or variable
        if(isAlpha(c)){
            string id;
            while(i<n && (isAlpha(expr[i]) || isdigit((unsigned char)expr[i]))) { id.push_back(expr[i]); i++; }
            // if next is '(' it's a function
            size_t j=i; while(j<n && isspace((unsigned char)expr[j])) j++;
            if(j<n && expr[j]=='(') out.emplace_back(T_FUNC, id);
            else out.emplace_back(T_VAR, id);
            continue;
        }
        // unknown char
        // treat as operator
        out.emplace_back(T_OP, string(1,c));
        i++;
    }
    return out;
}

// ----------------- Shunting Yard -> RPN -----------------
int prec(const string &op){
    if(op=="+"||op=="-") return 2;
    if(op=="*"||op=="/") return 3;
    if(op=="^") return 4;
    return 0;
}
bool isRightAssoc(const string &op){ return op=="^"; }

vector<Token> toRPN(const vector<Token>& tokens){
    vector<Token> out;
    vector<Token> st;
    for(size_t i=0;i<tokens.size();++i){
        Token t = tokens[i];
        if(t.type==T_NUM || t.type==T_VAR){
            out.push_back(t);
        } else if(t.type==T_FUNC){
            st.push_back(t);
        } else if(t.type==T_COMMA){
            while(!st.empty() && st.back().type!=T_LP){
                out.push_back(st.back()); st.pop_back();
            }
        } else if(t.type==T_OP){
            while(!st.empty() && st.back().type==T_OP &&
                (( !isRightAssoc(t.text) && prec(t.text) <= prec(st.back().text)) ||
                 ( isRightAssoc(t.text) && prec(t.text) < prec(st.back().text)))){
                out.push_back(st.back()); st.pop_back();
            }
            st.push_back(t);
        } else if(t.type==T_LP){
            st.push_back(t);
        } else if(t.type==T_RP){
            while(!st.empty() && st.back().type!=T_LP){
                out.push_back(st.back()); st.pop_back();
            }
            if(!st.empty() && st.back().type==T_LP) st.pop_back();
            if(!st.empty() && st.back().type==T_FUNC){
                out.push_back(st.back()); st.pop_back();
            }
        }
    }
    while(!st.empty()){
        out.push_back(st.back()); st.pop_back();
    }
    return out;
}

// ----------------- Environment -----------------
struct Env {
    unordered_map<string, Complex> vars;
    unordered_map<string, string> macros;
    vector<string> history;
    bool angle_deg = true; // default degrees for user convenience
} env;

// Helper parse number token to Complex
Complex parseNumberToken(const string &s){
    // forms: 123, 12.3, 1e3, 3i, 1.2e-3i
    string t = s;
    bool has_i = false;
    if(!t.empty() && t.back()=='i'){ has_i=true; t.pop_back(); }
    double v = 0.0;
    if(t.empty() || t=="+" || t=="-") {
        // like "i" or "+i" or "-i"
        v = (t=="-")?-1.0:1.0;
    } else {
        try {
            v = stod(t);
        } catch(...){
            v = 0.0;
        }
    }
    if(has_i) return Complex(0.0, v);
    else return Complex(v, 0.0);
}

// ----------------- Function evaluation -----------------
Complex callFunction(const string &fname, const vector<Complex> &args){
    string f = fname;
    for(auto &c: f) c = tolower(c);
    // unary functions
    if(f=="sin"){
        double x = args[0].real();
        if(!isfinite(args[0].imag())){} // ignore
        if(env.angle_deg) x = x * PI/180.0;
        return Complex(sin(x), 0.0);
    }
    if(f=="cos"){
        double x = args[0].real();
        if(env.angle_deg) x = x * PI/180.0;
        return Complex(cos(x), 0.0);
    }
    if(f=="tan"){
        double x = args[0].real();
        if(env.angle_deg) x = x * PI/180.0;
        return Complex(tan(x), 0.0);
    }
    if(f=="asin"){ double x=args[0].real(); double r = asin(x); if(env.angle_deg) r*=180.0/PI; return Complex(r,0); }
    if(f=="acos"){ double x=args[0].real(); double r = acos(x); if(env.angle_deg) r*=180.0/PI; return Complex(r,0); }
    if(f=="atan"){ double x=args[0].real(); double r = atan(x); if(env.angle_deg) r*=180.0/PI; return Complex(r,0); }
    if(f=="sqrt"){ return Complex(sqrt(args[0].real()), 0.0); }
    if(f=="abs"){ return Complex(abs(args[0]), 0.0); }
    if(f=="ln"){ return Complex(log(args[0].real()), 0.0); }
    if(f=="log" || f=="log10"){ return Complex(log10(args[0].real()), 0.0); }
    if(f=="exp"){ return Complex(exp(args[0].real()), 0.0); }
    if(f=="real"){ return Complex(args[0].real(), 0.0); }
    if(f=="imag"){ return Complex(args[0].imag(), 0.0); }
    if(f=="conj"){ return conj(args[0]); }
    if(f=="arg"){ return Complex(arg(args[0]), 0.0); }
    if(f=="pow" || f=="p"){
        double a=args[0].real(); double b=args[1].real();
        return Complex(pow(a,b),0.0);
    }
    if(f=="round"){ return Complex(round(args[0].real()),0.0); }
    // constants
    if(f=="pi") return Complex(PI,0);
    if(f=="e") return Complex(exp(1.0),0);
    // fallback
    return Complex(0.0,0.0);
}

// Evaluate RPN
optional<Complex> evalRPN(const vector<Token> &rpn){
    vector<Complex> st;
    for(const auto &t : rpn){
        if(t.type==T_NUM){
            st.push_back(parseNumberToken(t.text));
        } else if(t.type==T_VAR){
            string v = t.text;
            // variable or macro call
            auto it = env.vars.find(v);
            if(it!=env.vars.end()){
                st.push_back(it->second);
            } else {
                // try macros -> expand by tokenizing macro expression, evaluate (simple single-pass recursion limited)
                auto mit = env.macros.find(v);
                if(mit!=env.macros.end()){
                    string macroExpr = mit->second;
                    // naive: tokenize and evaluate separately
                    vector<Token> tok = tokenize(macroExpr);
                    vector<Token> r = toRPN(tok);
                    auto val = evalRPN(r);
                    if(!val) return nullopt;
                    st.push_back(*val);
                } else {
                    cerr << "Unknown variable/macro: " << v << "\n";
                    return nullopt;
                }
            }
        } else if(t.type==T_OP){
            string op = t.text;
            if(op=="+"){
                if(st.size()<2) return nullopt;
                auto b=st.back(); st.pop_back(); auto a=st.back(); st.pop_back();
                st.push_back(a+b);
            } else if(op=="-"){
                if(st.size()<2) return nullopt;
                auto b=st.back(); st.pop_back(); auto a=st.back(); st.pop_back();
                st.push_back(a-b);
            } else if(op=="*"){
                if(st.size()<2) return nullopt;
                auto b=st.back(); st.pop_back(); auto a=st.back(); st.pop_back();
                st.push_back(a*b);
            } else if(op=="/"){
                if(st.size()<2) return nullopt;
                auto b=st.back(); st.pop_back(); auto a=st.back(); st.pop_back();
                st.push_back(a/b);
            } else if(op=="^"){
                if(st.size()<2) return nullopt;
                auto b=st.back(); st.pop_back(); auto a=st.back(); st.pop_back();
                st.push_back(Complex(pow(a.real(), b.real()), 0.0));
            } else {
                cerr << "Unsupported op: " << op << "\n";
                return nullopt;
            }
        } else if(t.type==T_FUNC){
            string fname = t.text;
            // determine number of args by previous tokens? Simpler: support known arities
            // we'll support unary and binary functions; check stack size
            if(st.empty()) return nullopt;
            // guess arity: if function is pow -> 2 else 1
            int arity = (fname=="pow" || fname=="p")?2:1;
            if((int)st.size() < arity) return nullopt;
            vector<Complex> args(arity);
            for(int k=arity-1;k>=0;--k){ args[k] = st.back(); st.pop_back(); }
            Complex res = callFunction(fname, args);
            st.push_back(res);
        }
    }
    if(st.size()!=1) return nullopt;
    return st.back();
}

// ----------------- Unit conversions (small set) -----------------
optional<Complex> tryUnitConversion(const string &expr){
    // detect pattern number unit, e.g., "30deg", "100cm", "2m"
    string s = trim(expr);
    // scan for trailing alpha
    size_t pos = s.size();
    while(pos>0 && isalpha((unsigned char)s[pos-1])) pos--;
    if(pos==s.size()) return nullopt;
    string numPart = s.substr(0,pos);
    string unit = s.substr(pos);
    // handle complex forms skip
    try {
        double val = stod(numPart);
        if(unit=="deg"){ return Complex(val,0.0); }
        if(unit=="rad"){ if(env.angle_deg) return Complex(val*180.0/PI,0.0); else return Complex(val,0.0); }
        if(unit=="cm"){ return Complex(val/100.0,0.0); } // to meters
        if(unit=="mm"){ return Complex(val/1000.0,0.0); }
        if(unit=="m"){ return Complex(val,0.0); }
    } catch(...) { return nullopt; }
    return nullopt;
}

// ----------------- REPL helpers -----------------
void printHelp(){
    cout << "Casio++ tiny manual:\n";
    cout << " - Enter arithmetic expressions, e.g. 2+3*4, (1+2)^3\n";
    cout << " - Functions: sin, cos, tan, asin, acos, atan, sqrt, ln, log, exp, abs, pow\n";
    cout << " - Constants: pi, e\n";
    cout << " - Complex numbers: use 'i' e.g. 3+4i, 2i\n";
    cout << " - Variables: a = 3.5  then use a*2\n";
    cout << " - Macros: :macro name = expression  (creates reusable named formula)\n";
    cout << " - History: :history  and !n to run entry n\n";
    cout << " - Angle mode: :deg or :rad  (default deg). Also you can use 30deg explicitly.\n";
    cout << " - Other commands: :help, :vars, :macros, :units, exit\n";
}

void listVars(){
    cout << "Variables:\n";
    for(auto &p: env.vars){
        cout << " " << p.first << " = " << p.second << "\n";
    }
}
void listMacros(){
    cout << "Macros:\n";
    for(auto &p: env.macros){
        cout << " " << p.first << " = " << p.second << "\n";
    }
}
void listHistory(){
    cout << "History:\n";
    for(size_t i=0;i<env.history.size();++i){
        cout << i+1 << ": " << env.history[i] << "\n";
    }
}
void listUnits(){
    cout << "Supported quick units: deg, rad, m, cm, mm\n";
}

// ----------------- Main REPL -----------------
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout << "Casio++ (C++) interactive. Type :help\n";
    string line;
    while(true){
        cout << ">> ";
        if(!getline(cin, line)) break;
        line = trim(line);
        if(line.empty()) continue;
        env.history.push_back(line);

        if(line=="exit"||line=="quit") break;
        if(line==":help"){ printHelp(); continue; }
        if(line==":vars"){ listVars(); continue; }
        if(line==":macros"){ listMacros(); continue; }
        if(line==":history"){ listHistory(); continue; }
        if(line==":units"){ listUnits(); continue; }
        if(line==":deg"){ env.angle_deg=true; cout<<"Angle mode: degrees\n"; continue; }
        if(line==":rad"){ env.angle_deg=false; cout<<"Angle mode: radians\n"; continue; }
        // history execution !n
        if(line.size()>0 && line[0]=='!' ){
            string num = line.substr(1);
            try{
                int idx = stoi(num);
                if(idx>=1 && idx <= (int)env.history.size()){
                    string cmd = env.history[idx-1];
                    cout << "Running: " << cmd << "\n";
                    line = cmd;
                } else { cout<<"Invalid history index\n"; continue; }
            } catch(...){ cout<<"Invalid history syntax\n"; continue; }
        }

        // macro assignment :macro name = expr
        if(line.rfind(":macro", 0)==0){
            string t = trim(line.substr(6));
            size_t eq = t.find('=');
            if(eq==string::npos){ cout << "Macro syntax: :macro name = expression\n"; continue; }
            string name = trim(t.substr(0,eq));
            string expr = trim(t.substr(eq+1));
            if(name.empty() || expr.empty()){ cout << "Invalid macro definition\n"; continue; }
            env.macros[name] = expr;
            cout << "Macro saved: " << name << "\n";
            continue;
        }
        // variable assignment like a = expression
        size_t eqpos = string::npos;
        {
            // find '=' not inside parentheses
            int depth = 0;
            for(size_t i=0;i<line.size();++i){
                char c = line[i];
                if(c=='(') depth++;
                if(c==')') depth--;
                if(c=='=' && depth==0){ eqpos = i; break; }
            }
        }
        if(eqpos!=string::npos){
            string left = trim(line.substr(0, eqpos));
            string right = trim(line.substr(eqpos+1));
            if(!left.empty() && isAlpha(left[0])){
                // evaluate right
                // try unit conversion quick
                optional<Complex> maybeConv = tryUnitConversion(right);
                Complex val;
                if(maybeConv) val = *maybeConv;
                else {
                    vector<Token> tok = tokenize(right);
                    vector<Token> rpn = toRPN(tok);
                    auto res = evalRPN(rpn);
                    if(!res){ cout << "Evaluation error on RHS\n"; continue; }
                    val = *res;
                }
                env.vars[left] = val;
                cout << left << " = " << val << "\n";
                continue;
            }
        }

        // as-is evaluation
        // try unit conversion quick
        optional<Complex> maybeConv = tryUnitConversion(line);
        if(maybeConv){
            cout << *maybeConv << "\n";
            continue;
        }

        // handle direct macro call with arguments? Not implemented; macros are simple names.
        // Tokenize and run
        vector<Token> tok = tokenize(line);
        vector<Token> rpn = toRPN(tok);
        auto res = evalRPN(rpn);
        if(!res){
            cout << "Error: could not evaluate expression\n";
        } else {
            Complex v = *res;
            // pretty print: if imag zero print real
            if(abs(v.imag()) < 1e-12) cout << v.real() << "\n";
            else cout << v << "\n";
        }
    }
    cout << "Bye.\n";
    return 0;
}
