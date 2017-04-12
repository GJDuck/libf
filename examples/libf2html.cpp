/*
 * Copyright (c) The National University of Singapore.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <cassert>
#include <cctype>
#include <cstdio>

#include "../fstring.h"
#include "../ftuple.h"
#include "../fvector.h"

/*
 * Read an entire file as a string.
 */
static F::String read_file(const char *filename)
{
    FILE *stream = fopen(filename, "r");
    assert(stream != NULL);
    F::String s = F::string();
    while (!feof(stream) && !ferror(stream))
    {
        char buf[BUFSIZ];
        size_t size = fread(buf, sizeof(char), BUFSIZ-1, stream);
        if (size != 0)
        {
            buf[size] = '\0';
            assert(size == strlen(buf));
            s += buf;
        }
    }
    fclose(stream);
    return s;
}

/*
 * Parse the input.
 */
static PURE F::Vector<F::Tuple<F::Vector<F::String>, F::Vector<F::String>>>
parse_input(F::String input)
{
    auto output =
        F::vector<F::Tuple<F::Vector<F::String>, F::Vector<F::String>>>();

    F::StringItr i = begin(input), ie = end(input);

    while (i != ie)
    {
        // STEP #1: Find comment block:
        ssize_t j = 0;
        const char prefix[] = "/**\n";
        while (i != ie && prefix[j] != '\0')
        {
            if (*i != prefix[j])
                break;
            ++i;
            j++;
        }
        if (i == ie)
            break;
        if (prefix[j] != '\0')
        {
            i -= (j-1);
            continue;
        }

        // STEP #2: Parse comment:
        F::Vector<F::String> comment = F::vector<F::String>();
        while (true)
        {
            if (i == ie)
                goto parse_error;
            char32_t c = *i;
            ++i;
            if (c != ' ' || i == ie)
                goto parse_error;
            c = *i;
            ++i;
            if (c != '*' || i == ie)
                goto parse_error;
            c = *i;
            ++i;
            bool done = false;
            switch (c)
            {
                case ' ':
                {
                    F::String line = F::string();
                    while (i != ie && (c = *i) != '\n')
                    {
                        line += c;
                        ++i;
                    }    
                    if (i == ie)
                        goto parse_error;
                    comment = push_back(comment, line);
                    ++i;
                    break;
                }
                case '/':
                    while (i != ie && (c = *i) != '\n')
                        ++i;
                    if (i != ie)
                        ++i;
                    done = true;
                    break;
                default:
                    goto parse_error;
            }
            if (done)
                break;
        }

        // STEP #3: Parse the function prototype:
        F::Vector<F::String> prototype = F::vector<F::String>();
        while (true)
        {
            if (i == ie)
                goto parse_error;
            char32_t c = *i;
            ++i;
            if (c == '{' || c == '\n')
                break;
            F::String line = F::string(c);
            while (i != ie && (c = *i) != '\n')
            {
                line += c;
                ++i;
            }
            if (i == ie)
                goto parse_error;
            prototype = push_back(prototype, line);
            ++i;
        }

        auto entry =
            F::tuple<F::Vector<F::String>, F::Vector<F::String>>
                (comment, prototype);
        output = push_back(output, entry);
    }

    return output;

parse_error:
    fprintf(stderr, "parse error!\n");
    abort();
}

/*
 * Converts a header file into html.
 */
int main(int argc, char **argv)
{
    assert(argc == 2);
    F::String name0 = F::string(argv[1]);
    while (true)
    {
        auto idx = find(name0, '/');
        if (empty(idx))
            break;
        name0 = right(name0, idx+1);
    }
    name0 = right(name0, 1);
    auto idx = find(name0, '.');
    if (!empty(idx))
        name0 = left(name0, idx);
    const char *name = c_str(name0);
    F::String input = read_file(argv[1]);
    auto output = parse_input(input);

    output = F::map<F::Tuple<F::Vector<F::String>, F::Vector<F::String>>>(
        output,
        [](size_t idx,
           F::Tuple<F::Vector<F::String>, F::Vector<F::String>> entry) ->
           F::Tuple<F::Vector<F::String>, F::Vector<F::String>>
        {
            return tuple(
                F::map<F::String>(first(entry),
                [](size_t idx, F::String line) -> F::String
                {
                    line = F::replace_all(line, "&", F::string("&amp;"));
                    line = F::replace_all(line, "<", F::string("&lt;"));
                    line = F::replace_all(line, ">", F::string("&gt;"));
                    return line;
                }),
                F::map<F::String>(second(entry),
                [](size_t idx, F::String line) -> F::String
                {
                    line = F::replace_all(line, "PURE ", F::string("pure "));
                    line = F::replace_all(line, "extern ", F::string());
                    line = F::replace_all(line, "inline ", F::string());
                    line = F::replace_all(line, ");", F::string(")"));
                    line = F::replace_all(line, " _", F::string(" "));
                    line = F::replace_all(line, "*_", F::string("*"));
                    line = F::replace_all(line, "&_", F::string("&"));
                    line = F::replace_all(line, "<_", F::string("<"));
                    line = F::replace_all(line, "&", F::string("&amp;"));
                    line = F::replace_all(line, "<", F::string("&lt;"));
                    line = F::replace_all(line, ">", F::string("&gt;"));
                    line = F::replace_all(line, "pure",
                        F::string("<b>pure</b>"));
                    line = F::replace_all(line, "const ",
                        F::string("<b>const</b> "));
                    line = F::replace_all(line, "constexpr ",
                        F::string("<b>constexpr</b> "));
                    line = F::replace_all(line, "void ",
                        F::string("<b>void</b> "));
                    line = F::replace_all(line, "bool ",
                        F::string("<b>bool</b> "));
                    line = F::replace_all(line, "char ",
                        F::string("<b>char</b> "));
                    line = F::replace_all(line, "short ",
                        F::string("<b>short</b> "));
                    line = F::replace_all(line, "int ",
                        F::string("<b>int</b> "));
                    line = F::replace_all(line, "long ",
                        F::string("<b>long</b> "));
                    line = F::replace_all(line, "unsigned ",
                        F::string("<b>unsigned</b> "));
                    line = F::replace_all(line, "signed ",
                        F::string("<b>signed</b> "));
                    line = F::replace_all(line, "float ",
                        F::string("<b>float</b> "));
                    line = F::replace_all(line, "double ",
                        F::string("<b>double</b> "));
                    line = F::replace_all(line, "template ",
                        F::string("<b>template</b> "));
                    line = F::replace_all(line, "typename ",
                        F::string("<b>typename</b> "));
                    line = F::replace_all(line, "typename.",
                        F::string("<b>typename</b>."));
                    line = F::replace_all(line, "operator",
                        F::string("<b>operator</b>"));
                    line = F::replace_all(line, "ssize_t ",
                        F::string("<b>ssize_t</b> "));
                    line = F::replace_all(line, "size_t ",
                        F::string("<b>size_t</b> "));
                    line = F::replace_all(line, "char32_t ",
                        F::string("<b>char32_t</b> "));
                    line = F::replace_all(line, "String ",
                        F::string("<b><u>String</u></b> "));
                    line = F::replace_all(line, "StringItr ",
                        F::string("<b><u>StringItr</u></b> "));
                    line = F::replace_all(line, "Vector&",
                        F::string("<b><u>Vector</u></b>&"));
                    line = F::replace_all(line, "VectorItr&",
                        F::string("<b><u>VectorItr</u></b>&"));
                    line = F::replace_all(line, "List&",
                        F::string("<b><u>List</u></b>&"));
                    line = F::replace_all(line, "ListItr&",
                        F::string("<b><u>ListItr</u></b>&"));
                    line = F::replace_all(line, "Set&",
                        F::string("<b><u>Set</u></b>&"));
                    line = F::replace_all(line, "SetItr&",
                        F::string("<b><u>SetItr</u></b>&"));
                    line = F::replace_all(line, "Map&",
                        F::string("<b><u>Map</u></b>&"));
                    line = F::replace_all(line, "MapItr&",
                        F::string("<b><u>MapItr</u></b>&"));
                    line = F::replace_all(line, "Tuple&",
                        F::string("<b><u>Tuple</u></b>&"));
                    line = F::replace_all(line, "Maybe&",
                        F::string("<b><u>Maybe</u></b>&"));
                    line = F::replace_all(line, "Optional&",
                        F::string("<b><u>Optional</u></b>&"));
                    line = F::replace_all(line, "Union&",
                        F::string("<b><u>Union</u></b>&"));
                    line = F::replace_all(line, "Result&",
                        F::string("<b><u>Result</u></b>&"));
                    return line;
                }));
        });

    F::String out = F::string();
    out += "<html>\n";
    out += "<body>\n";
    out += "<h1>";
    out += F::map(name0,
        [](size_t idx, char32_t c) -> char32_t
        {
            return toupper(c);
        });
    out += "</h1>\n";
    out += "<pre style=\"background-color: #FFFFBB;\">\n";
    unsigned count = 0;
    for (auto entry: output)
    {
        bool prev = false;
        out += "<a href=\"#function_";
        out += name0;
        out += '_';
        out += F::show(count);
        out += "\" style=\"text-decoration:none;\">";
        count++;
        for (auto line: second(entry))
        {
            if (prev)
                out += ' ';
            prev = true;
            out += line;
        }
        out += ";</a>\n";
    }
    out += "</pre>\n";
    count = 0;
    for (auto entry: output)
    {
        out += "<hr>\n";
        out += "<pre style=\"background-color: #FFFFBB;\">";
        out += "<a name=\"function_";
        out += name0;
        out += '_';
        out += F::show(count);
        out += "\">";
        count++;
        for (auto line: second(entry))
        {
            out += line;
            out += '\n';
        }
        out += "</a></pre>\n";
        out += "<p>\n";
        for (auto line: first(entry))
        {
            out += '\t';
            out += line;
            out += '\n';
        }
        out += "</p>\n";
    }
    out += "</body>\n";
    out += "</html>\n";

    printf("%s\n", c_str(out));

    return 0;
}

