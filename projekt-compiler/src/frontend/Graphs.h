#ifndef GRAPHS_H
#define GRAPHS_H

#include <map>
#include <locale>         // std::locale, std::isdigit
#include <fstream>
#include <sstream>

#include <frontend/adjacency_list.h>
#include <frontend/Invalid_Format_Exception.h>
#include <frontend/IO_Exception.h>

class Graphs
{
  public:
    typedef std::shared_ptr<Node> Node_ptr;
    typedef std::shared_ptr<Graph> Graph_ptr;
    typedef std::map<std::string, Graph_ptr> Graph_map;
    typedef const std::string& str;
    // static const std::regex regexs[];
  private:
    Graph_map graphs;
  public:
    Graphs();
    virtual ~Graphs();

    virtual bool put(str key, Graph_ptr graph);
    virtual Graph_ptr find(str key);
    virtual Graph_map::iterator begin();
    virtual Graph_map::iterator end();
    virtual size_t size();
    virtual void marshall(str file);
    virtual void unmarshall(str file, char delimeter);
    void writeGraphViz(Graphs::str file);
  private:
    Node_ptr findNode(Graph_ptr adj, std::string id);
    Command getCommand(std::string& cmd);
    Node_ptr unmarshall_line(Graph_ptr adj, std::string& line, char delimiter);
    Graph_ptr unmarshall_Function(std::ifstream& infile, std::string& line, char delimiter);
    void skip_empty_lines(std::ifstream& infile, std::string& line);
    bool containsFunctionName(str line);
    bool containsBeginAndEndChar(std::string& cmd, char begin, char end);
};
void printNode(Graphs::Node_ptr n);
#endif // GRAPHS_H
