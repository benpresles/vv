/*
 * File automatically generated by
 * gengen 1.4.1 by Lorenzo Bettini 
 * http://www.gnu.org/software/gengen
 */

#ifndef CLEAR_GIVEN_GEN_CLASS_H
#define CLEAR_GIVEN_GEN_CLASS_H

#include <string>
#include <iostream>

using std::string;
using std::ostream;

class clear_given_gen_class
{
 protected:
  string arg_struct;
  bool group;
  string var_arg;

 public:
  clear_given_gen_class() :
    group (false)
  {
  }
  
  clear_given_gen_class(const string &_arg_struct, bool _group, const string &_var_arg) :
    arg_struct (_arg_struct), group (_group), var_arg (_var_arg)
  {
  }

  static void
  generate_string(const string &s, ostream &stream, unsigned int indent)
  {
    if (!indent || s.find('\n') == string::npos)
      {
        stream << s;
        return;
      }

    string::size_type pos;
    string::size_type start = 0;
    string ind (indent, ' ');
    while ( (pos=s.find('\n', start)) != string::npos)
      {
        stream << s.substr (start, (pos+1)-start);
        start = pos+1;
        if (start+1 <= s.size ())
          stream << ind;
      }
    if (start+1 <= s.size ())
      stream << s.substr (start);
  }

  void set_arg_struct(const string &_arg_struct)
  {
    arg_struct = _arg_struct;
  }

  void set_group(bool _group)
  {
    group = _group;
  }

  void set_var_arg(const string &_var_arg)
  {
    var_arg = _var_arg;
  }

  void generate_clear_given(ostream &stream, unsigned int indent = 0);
  
};

#endif // CLEAR_GIVEN_GEN_CLASS_H
