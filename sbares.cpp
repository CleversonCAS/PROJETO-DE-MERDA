/*!
 * Converts an arithmetic expression in infix notation into postfix
 * representation. After the convertion is successfyly done, the
 * program evaluates the expression based on stacks and queues.
 *
 * Input Postfix expression must be in a desired format.
 * Operands and operator, **both must be single character** (all positives).
 * Only '+', '-', '*', '%', '/', and '^' (for exponentiation) operators are expected;
 * Any other character is just ignored.
 */

#include <iostream>  // cout, endl
#include <stack>     // stack
#include <string>    // string
#include <cassert>   // assert
#include <cmath>     // pow
#include <stdexcept> // std::runtime_error
#include <vector>    // std::vector
#include "parser.h"
#include <iomanip>

//=== Aliases
using value_type = long int; //!< Type we operate on.
using symbol = char; //!< A symbol in this implementation is just a char.

// Simple helper functions that identify the incoming symbol.
bool is_operator( symbol s )
{
    return std::string("*^/%+-").find( s ) != std::string::npos;

    /*
    std::string  target("*^/%+-");

    auto result = target.find( s );

    if ( result == std::string::npos ) return false;
    else return true;
    */

    /*

    switch( s )
    {
        case '*':
        case '/':
            ...
        return true;
        default: return false;
    }
    */
}

bool is_operand( symbol s )
{   return s >= '0' and s <= '9';   }

bool is_opening_scope( symbol s )
{ return s == '('; }

bool is_closing_scope( symbol s )
{ return s == ')'; }

/// Check the operand's type of association.
bool is_right_association( symbol op )
{ return op == '^'; }

/// Converts a expression in infix notation to a corresponding profix representation.
std::string infix_to_postfix( std::string );

/// Converts a char (1-digit operand) into an integer.
value_type char2integer( char c )
{ return c - '0'; }

/// Change an infix expression into its corresponding postfix representation.
value_type evaluate_postfix( std::string );

/// Returns the precedence value (number) associated with an operator.
short get_precedence( symbol op )
{
    switch( op )
    {
        case '^' : return 3;
        case '*' :
        case '/' :
        case '%' : return 2;
        case '+' :
        case '-' : return 1;
        case '(' : return 0;
        default  : assert( false );  return -1;
    }
}

/// Determines whether the first operator is >= than the second operator.
bool has_higher_or_eq_precedence( symbol op1 , symbol op2 )
{
    // pega os valores numericos correspondentes aas precedencias.
    int p_op1 = get_precedence( op1 );
    int p_op2 = get_precedence( op2 );

    if ( p_op1 > p_op2 )
    {
        return true;
    }
    else if ( p_op1 < p_op2 )
    {
        return false;
    }
    else // p_op1 == p_op2 
    {
        // exceção a regra, que acontece com associacao direita-esquerda.
        if ( is_right_association( op1 ) ) return false;
        // regra normal.
        return true;
    }
}

/// Execute the binary operator on two operands and return the result.
value_type execute_operator( value_type v1, value_type v2, symbol op )
{
    switch( op )
    {
        case '^':  return pow( v1,v2 );
        case '*':  return v1*v2;
        case '/':  if ( v2 == 0 ) throw std::runtime_error( "divisao por zero" );
                   return v1/v2;
        case '%':  if ( v2 == 0 ) throw std::runtime_error( "divisao por zero" );
                   return v1%v2;
        case '+': return v1+v2;
        case '-': return v1-v2;
        default: throw std::runtime_error( "undefined operator" );
    }
}

void print_error_msg( const Parser::ResultType & result, std::string str )
{
    std::string error_indicator( str.size()+1, ' ');

    // Have we got a parsing error?
    error_indicator[result.at_col] = '^';
    switch ( result.type )
    {
        case Parser::ResultType::UNEXPECTED_END_OF_EXPRESSION:
            std::cout << ">>> Unexpected end of input at column (" << result.at_col << ")!\n";
            break;
        case Parser::ResultType::ILL_FORMED_INTEGER:
            std::cout << ">>> Ill formed integer at column (" << result.at_col << ")!\n";
            break;
        case Parser::ResultType::MISSING_TERM:
            std::cout << ">>> Missing <term> at column (" << result.at_col << ")!\n";
            break;
        case Parser::ResultType::EXTRANEOUS_SYMBOL:
            std::cout << ">>> Extraneous symbol after valid expression found at column (" << result.at_col << ")!\n";
            break;
        case Parser::ResultType::INTEGER_OUT_OF_RANGE:
            std::cout << ">>> Integer constant out of range beginning at column (" << result.at_col << ")!\n";
            break;
        default:
            std::cout << ">>> Unhandled error found!\n";
            break;
    }

    std::cout << "\"" << str << "\"\n";
    std::cout << " " << error_indicator << std::endl;
}


std::string infix_to_postfix( std::string infix )
{
    std::string postfix(""); // resultado da conversao.
    std::stack< symbol > s; // pilha de ajuda na conversao.

    // Percorrer a entrada, para processar cada item/token/caractere
    for( auto c : infix )
    {
        //std::cout << "\n>>> Infix: \"" << infix << "\"\n";
        //std::cout << ">>> Processando o \'" << c << "\'\n";
        // Operando vai direto pra saida.
        if( is_operand( c ) )
            postfix += c;
        else if( is_opening_scope(c) )
            s.push( c ); // '(' entra na pilha de espera em cima de quem estiver por la.
        else if( is_closing_scope(c) )
        {
            // desempilhe ateh achar o escopo de abertura correspondente.
            while( not is_opening_scope( s.top() ) )
            {
                postfix += s.top();
                s.pop();
            }
            s.pop(); // Lembre de descartar o '(' que estah no topo da pilha.
        }
        else if ( is_operator( c ) ) // + - ^ *...
        {
            // Desempilhar as operaçoes em espera que são iguais ou superiores
            // em prioridade (com excecao da associacao direita-esquerda).
            while( not s.empty() and has_higher_or_eq_precedence( s.top(), c ) ) // s.top() >= c
            {
                postfix += s.top();
                s.pop();
            }

            // A operacao que chegar, sempre tem que esperar.
            s.push( c );
        }
        else // qualquer coisa que não seja o que eu quero. Ex.: espaço em branco.
        {
            // ignoro, não faço nada.
        }
        //std::cout << ">>> Posfix: \"" << postfix << "\"\n";
    }

    // Lembre-se de descarregar as operacoes pendentes da pilha.
    while( not s.empty() )
    {
        postfix += s.top();
        s.pop();
    }

    return postfix;
}

value_type evaluate_postfix( std::string postfix )
{
    std::stack< value_type > s;

    for( auto c : postfix )
    {
        if ( is_operand( c ) )
            s.push( char2integer( c ) );
        else if ( is_operator( c ) )
        {
            value_type op2 = s.top(); s.pop();
            value_type op1 = s.top(); s.pop();
            auto result = execute_operator( op1, op2, c ); // ( 2, 9, '*' )
            s.push( result );
        }
        else assert( false );
    }

    return s.top();
}
int main( void )
{
    // A expression is a queue of sysmbols (chars).
    std::vector< std::string > exps {
        "4/(5^2)+(6^2^3)",
        "1+ 3 * ( 4 + 8 * 3 ^7)",
        "2*2*3",
        "2^2^3",
        "22+1",
        "4 + 03",
        "10",
        "    12    +    4   8",
        "32767 - 32768 + 3",
        "5 + -32766",
        "5 + -32769",
        "12 + 3",
        "-3+-5+-6",
        "12 + 3     -3 + -34 ",
        "+12",
        "1.3 * 4",
        "a + 4",
        "       ",
        "  123 *  548",
        "4 + ",
        "32a23",
        "43 + 54 -   ",
        "1234567888889974232214433432 + 4",
        "10 / 2",
        "2^2^3",
        "2 + (3 - 4)"
         } ;

    Parser my_parser; // Instancia um parser.
    // Tentar analisar cada expressão da lista.
    for( const auto & expr : exps )
    {
        // Fazer o parsing desta expressão.
        auto result = my_parser.parse( expr );
        // Preparar cabeçalho da saida.
        std::cout << std::setfill('=') << std::setw(80) << "\n";
        std::cout << std::setfill(' ') << ">>> Parsing \"" << expr << "\"\n";
        // Se deu pau, imprimir a mensagem adequada.
        if ( result.type != Parser::ResultType::OK )
            print_error_msg( result, expr );
        else
            std::cout << ">>> Expression SUCCESSFULLY parsed!\n";

         // Recuperar a lista de tokens.
        auto lista = my_parser.get_tokens();
        std::cout << ">>> Tokens: { ";
        std::copy( lista.begin(), lista.end(),
                std::ostream_iterator< Token >(std::cout, " ") );
        std::cout << "}\n";
        std::cout << std::endl;
    }

    for ( const auto& e : exps )
    {
        auto postfix = infix_to_postfix( e );
        std::cout << ">>> Input (infix)    = " << e << "\n";
        std::cout << ">>> Output (postfix) = " << postfix << "\n";

        auto result = evaluate_postfix( postfix );
        std::cout << ">>> Result is: " << result << std::endl;
    }

    std::cout << "\n>>> Normal exiting...\n";
    return EXIT_SUCCESS;
}
