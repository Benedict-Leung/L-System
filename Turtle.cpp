#define _USE_MATH_DEFINES
#include <vector>
#include <stack>
#include <string>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <iostream>
#include <algorithm>

struct Rule {
    std::string variable;
    std::string rule;
};

class Turtle {
    private:
        float step;
        float angle;

        glm::vec3 position = glm::vec3(0.0);

        glm::vec3 H = glm::vec3(0.0, 1.0, 0.0);
        glm::vec3 L = glm::vec3(-1.0, 0.0, 0.0);
        glm::vec3 U = glm::vec3(0.0, 0.0, 1.0);

        std::stack<glm::vec3> stack;
        std::vector<std::string> rules;
        std::string axiom;
        std::string condition;

    public:
        std::vector<GLfloat> vArray;

        Turtle(std::string _axiom = "", float _step = 1, float _angle = 0.0) {
            axiom = _axiom;
            condition = _axiom;
            step = _step;
            angle = _angle;

            vArray.push_back(0);
            vArray.push_back(0);
            vArray.push_back(0);
        }

        void reset() {
            position = glm::vec3(0.0);
            H = glm::vec3(0.0, 1.0, 0.0);
            L = glm::vec3(-1.0, 0.0, 0.0);
            U = glm::vec3(0.0, 0.0, 1.0);

            while (!stack.empty())
                stack.pop();

            vArray.clear();
            vArray.push_back(0);
            vArray.push_back(0);
            vArray.push_back(0);
        }
        
        void setAxiom(std::string a) {
            axiom = a;
            condition = a;
        }

        void setAngle(float a) {
            angle = a;
        }

        void setStep(float s) {
            step = s;
        }

        void clearRules() {
            rules.clear();
        }

        void addRule(std::string rule) {
            rules.push_back(rule);
        }
        
        std::string getCondition() {
            return condition;
        }

        void move(float distance) {
            position += H * step;

            vArray.push_back(position.x);
            vArray.push_back(position.y);
            vArray.push_back(position.z);
        }

        void updateParam(glm::mat3 V2) {
            H = V2[0];
            L = V2[1];
            U = V2[2];
        }

        void turnLeft() {
            glm::mat3 R, V;

            R = glm::mat3(
                cos(angle * M_PI / 180), sin(angle * M_PI / 180), 0.0,
                -sin(angle * M_PI / 180), cos(angle * M_PI / 180), 0.0,
                0.0, 0.0, 1.0
            );

            V = glm::mat3(H, L, U);

            updateParam(V * R);
        }

        void turnRight() {
            glm::mat3 R, V;

            R = glm::mat3(
                cos(-angle * M_PI / 180), sin(-angle * M_PI / 180), 0.0,
                -sin(-angle * M_PI / 180), cos(-angle * M_PI / 180), 0.0,
                0.0, 0.0, 1.0
            );

            V = glm::mat3(H, L, U);

            updateParam(V * R);
        }

        void pitchDown() {
            glm::mat3 R, V;

            R = glm::mat3(
                cos(angle * M_PI / 180), 0.0, -sin(angle * M_PI / 180),
                0.0, 1.0, 0.0,
                sin(angle * M_PI / 180), cos(angle * M_PI / 180), 0.0
            );

            V = glm::mat3(H, L, U);

            updateParam(V * R);
        }

        void pitchUp() {
            glm::mat3 R, V;

            R = glm::mat3(
                cos(-angle * M_PI / 180), 0.0, -sin(-angle * M_PI / 180),
                0.0, 1.0, 0.0,
                sin(-angle * M_PI / 180), 0.0, cos(-angle * M_PI / 180)
            );

            V = glm::mat3(H, L, U);

            updateParam(V * R);
        }

        void rollLeft() {
            glm::mat3 R, V;

            R = glm::mat3(
                1.0, 0.0, 0.0,
                0.0, cos(angle * M_PI / 180), -sin(angle * M_PI / 180),
                0.0, sin(angle * M_PI / 180), cos(angle * M_PI / 180)
            );

            V = glm::mat3(H, L, U);

            updateParam(V * R);
        }

        void rollRight() {
            glm::mat3 R, V;

            R = glm::mat3(
                1.0, 0.0, 0.0,
                0.0, cos(-angle * M_PI / 180), -sin(-angle * M_PI / 180),
                0.0, sin(-angle * M_PI / 180), cos(-angle * M_PI / 180)
            );

            V = glm::mat3(H, L, U);

            updateParam(V * R);
        }

        void turnAround() {
            glm::mat3 R, V;

            R = glm::mat3(
                cos(M_PI), sin(M_PI), 0.0,
                -sin(M_PI), cos(M_PI), 0.0,
                0.0, 0.0, 1.0
            );

            V = glm::mat3(H, L, U);

            updateParam(V * R);
        }

        std::vector<GLfloat>* interpret(std::string result) {
            reset();

            for (unsigned int i = 0, size = result.size(); i < size; ++i) {
                switch (result[i]) {
                    case '+':
                        turnLeft();
                        break;
                    case '-':
                        turnRight();
                        break;
                    case '&':
                        pitchDown();
                        break;
                    case '^':
                        pitchUp();
                        break;
                    case '\\':
                        rollLeft();
                        break;
                    case '/':
                        rollRight();
                        break;
                    case '|':
                        turnAround();
                        break;
                    case '[':
                        stack.push(position);
                        stack.push(H);
                        stack.push(L);
                        stack.push(U);
                        break;
                    case ']':
                        U = stack.top();
                        stack.pop();
                        L = stack.top();
                        stack.pop();
                        H = stack.top();
                        stack.pop();
                        position = stack.top();
                        stack.pop();
                        vArray.push_back(NAN);
                        vArray.push_back(NAN);
                        vArray.push_back(NAN);
                        vArray.push_back(position.x);
                        vArray.push_back(position.y);
                        vArray.push_back(position.z);
                        break;
                    default:
                        move(step);
                        break;
                }
            }
            return &vArray;
        }

        std::string iterate(const int generations = 1) {
            condition = axiom;
            std::vector<Rule> rul = getRules();

            for (auto i = 0; i < generations; i++) {
                std::string newCondition;

                for (auto j = 0; j < condition.size(); j++) {
                    std::string cur;
                    cur += condition[j];
                    std::string replacement = cur;

                    for (auto r : rul) {
                        if (cur == r.variable) {
                            replacement = r.rule;
                            break;
                        }
                    }
                    newCondition += replacement;
                }
                condition = newCondition;
            }
            return condition;
        }

        std::vector<Rule> getRules() {
            std::vector<Rule> listOfRules;

            for (auto r : rules) {
                auto pos = r.find(" -> ");

                if (pos != -1 && pos == 1) {
                    listOfRules.push_back(Rule({ r.substr(0, pos), r.substr(pos + 4) }));
                }
            }
            return listOfRules;
        }
};
