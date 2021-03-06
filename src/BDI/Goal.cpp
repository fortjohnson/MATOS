//
// Created by David Johnson on 4/18/17.
//

#include "Goal.h"

bool is_operator(const string& token){
    return token == "~=" || token == "<" || token == ">" || token == "==" || token == "!=" || token == ">=" || token == "<=" || token == "and" || token == "or" || token == "+" || token == "*" || token == "/" || token == "-";
}

int precedence(const string& token){

    if (token == "*" || token == "/") return 3;
    if (token == "+" || token == "-") return 2;
    if (token == "~=" || token == "<" || token == ">" || token == "==" || token == ">=" || token == "<=" || token == "!=") return 1;
    if (token == "and" || token == "or") return 0;
    else throw exception();
}

int evaluateArith(int operand1, int operand2, string op){

    if (op == "+") return operand2 + operand1;
    if (op == "-") return operand2 - operand1;
    if (op == "*") return operand2 * operand1;
    if (op == "/") return operand2 / operand1;

    return 0;
}

bool evaluateCondition(int operand1, int operand2, string op){

    if (op == "<")      return operand2 < operand1;
    if (op == "<=")     return operand2 <= operand1;
    if (op == "==")     return operand2 == operand1;
    if (op == "!=")     return operand2 != operand1;
    if (op == ">=")     return operand2 >= operand1;
    if (op == ">")      return operand2 > operand1;
    if (op == "and")    return operand2 && operand1;
    if (op == "or")     return operand2 || operand1;

    if(op == "~="){
        return abs(operand1 - operand2) < 1.5;
    }

    return true;
}


Goal::Goal(ActionFunction callback) :
    callback(callback)
{}

Goal::Goal(vector<string> infixExpression, ActionFunction callback) :
    callback(callback)
{
    setExpression(infixExpression);
}


void Goal::setExpression(vector<string> infixExpression) {

    stack<string> operatorStack;

    for (const string& token : infixExpression){
        // Check of operator is an operator
        if (is_operator(token)) {

                while (!operatorStack.empty() && precedence(token) <= precedence(operatorStack.top()) ){
                    outputQueue.push_back(operatorStack.top());
                    operatorStack.pop();
                }
                operatorStack.push(token);
        }
        // if not an operator than must be a variable or number
        else{
            outputQueue.push_back(token);
        }
    }

    while(!operatorStack.empty()){
        outputQueue.push_back(operatorStack.top());
        operatorStack.pop();
    }
}


bool Goal::evaluate(map<string, float>& params){

    if (outputQueue.empty())
        throw exception();

    stack<string> evalStack;
    queue<string, deque<string>> evalQueue(deque<string>(outputQueue.begin(), outputQueue.end()));

    while (!evalQueue.empty()){

        string test = evalQueue.front();

        if (!is_operator(evalQueue.front())){
            evalStack.push(evalQueue.front());
            evalQueue.pop();
        }
        else{
            string operand1 = evalStack.top();
            evalStack.pop();
            string operand2 = evalStack.top();
            evalStack.pop();

            string op = evalQueue.front();
            evalQueue.pop();

            int o1;
            int o2;



            try {
                o1 = int(params.at(operand1));
            }
            catch(exception e){
                try {
                    o1 = stoi(operand1);
                }
                catch(const exception& e){
                    if(operand1 == "true")          o1 = true;
                    else if(operand1 == "false")    o1 = false;
                    else {
                        stringstream error;
                        error << "Missing Parameter: " << operand1;
                        throw MissingParameterException(error.str().c_str());
                    }
                }
            }

            try {
                o2 = int(params.at(operand2));
            }
            catch(const exception& e){
                try {
                    o2 = stoi(operand2);
                }
                catch(const exception& e){
                    if(operand2 == "true")          o2 = true;
                    else if(operand2 == "false")    o2 = false;
                    else{
                        stringstream error;
                        error << "Missing Parameter: " << operand2;
                        throw MissingParameterException(error.str().c_str());
                    }
                }
            }


            if (op == "+" || op =="-" || op == "*" || op == "/"){
                int value = evaluateArith(o1, o2, op);
                evalStack.push(to_string(value));
            }
            else{
                bool result = evaluateCondition(o1, o2, op);
                evalStack.push(result ? "true" : "false");
            }
        }
    }

    if (evalStack.size() > 1)
        throw exception();

    string result = evalStack.top();

    if(result == "true")
        return true;

    return false;

}

void Goal::action(map<string,float>& params) {
    callback(evaluate(params), *this, params);
}


ostream&operator<<(ostream& os, const Goal& goal){

    for (const string& token : goal.outputQueue){
        os << token << " " ;
    }
    return os;
}


