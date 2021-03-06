#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <vector>
#include <chrono>
#include <limits>
#include <algorithm>
#include <map>

namespace
{
    using vertex = std::uint64_t;
    using vertex_array = std::vector<vertex>;
    using vertex_matrix = std::vector<vertex_array>;
    using _chrono = std::chrono::steady_clock;

    struct clique
    {
        vertex_array m_vertices = {};
        vertex_array m_candidates = {};
    };


    static vertex_matrix adjacency_matrix = {};
    static clique optimal_clique = {};

    static double time_limit = 0;

    static auto start_time = _chrono::now();

    std::vector<std::string> split(std::string& s, const std::string& delim)
    {
        std::vector<std::string> out = {};
        std::stringstream ss(s);
        std::size_t pos = 0;
        std::string token;
        while ((pos = s.find(delim)) != std::string::npos) {
            token = s.substr(0, pos);
            out.push_back(token);
            s.erase(0, pos + delim.size());
        }
        out.push_back(s);
        return out;
    }

    inline vertex_array get_connected(vertex v, vertex start_index = 0)
    {
        vertex n_vertices = static_cast<vertex>(adjacency_matrix.size());
        const auto& row = adjacency_matrix[v];
        vertex_array C = {};
        // TODO: verify if can calculate from v + 1:
        for (vertex i = start_index; i < n_vertices; ++i)
        {
            if (row[i] > 0) { C.push_back(i); }
        }
        return C;
    }

    vertex_array find_candidates(const clique& clq, vertex vertex_to_be_added)
    {
        vertex_array out = {};
        auto connected = get_connected(vertex_to_be_added);
        for (const auto& known_candidate : clq.m_candidates)
        {
            for (const auto& possible_candidate : connected)
            {
                if (possible_candidate == known_candidate)
                    out.push_back(possible_candidate);
            }
        }

        return out;
    }

    inline std::uint64_t colors(const vertex_array& vertices)
    {
        auto size = vertices.size();
        if (size <= 0) return 0;
        std::map<vertex, int> colors;

        for (const auto& vertex : vertices)
        {
            std::vector<int> neighbour_colors;
            for (const auto& neighbour : get_connected(vertex))
            {
                neighbour_colors.push_back(colors[neighbour]);
            }

            bool vertex_colored = false;
            int supposed_color = 1;
            while (vertex_colored != true)
            {
                bool color_of_neighbour = false;
                for (const auto& color : neighbour_colors)
                    if (color == supposed_color)
                    {
                        color_of_neighbour = true;
                        break;
                    }
                if (color_of_neighbour)
                {
                    supposed_color++;
                    continue;
                }
                colors[vertex] = supposed_color;
                vertex_colored = true;
            }
        }
        return std::max_element(colors.begin(), colors.end())->second;
    }

    inline std::uint64_t upper_bound(const clique& Q)
    {
        return Q.m_vertices.size() + colors(Q.m_candidates);
    }

    void max_clique(const clique& Q)
    {
        auto ub = upper_bound(Q);
        if (ub <= optimal_clique.m_vertices.size()) return;
        if (Q.m_candidates.size() == 0)
        {
            optimal_clique = Q;
            return;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(_chrono::now() - start_time);
        if (elapsed.count() > time_limit)
        {
            throw std::runtime_error("Out of time");
        }

        for (const auto& candidate : Q.m_candidates)
        {
            auto temp_q = Q;
            temp_q.m_candidates = find_candidates(temp_q, candidate);
            temp_q.m_vertices.push_back(candidate);
            max_clique(temp_q);
        }
    }

    std::string pretty_print(const clique& Q)
    {
        std::string s;
        for (const auto& vertex : Q.m_vertices)
        {
            s.append(std::to_string(vertex + 1));
            s.append(" ");
        }
        return s;
    }

#define ERROR_OUT(msg) std::cerr << msg << std::endl;
}

int main(int argc, char* argv[]) try
{
    if (argc < 3)
    {
        ERROR_OUT("Command-line arguments: <file> <time limit>. Ex: ./mlp graph.clq 1000");
        return 1;
    }
    std::ifstream f(argv[1]);
    if (!f.good())
    {
        ERROR_OUT("File is unreachable/not found");
        return 1;
    }
    time_limit = std::atof(argv[2]); // in seconds
    if (time_limit == 0)
    {
        ERROR_OUT("Time limit is incorrect");
        return 1;
    }

    std::string line;
    vertex n_vertices = 0;
//    std::size_t n_edges = 0;
    static constexpr char default_delim[] = " ";
    std::map<vertex, std::size_t> vertex_degrees_map;
    while (!f.eof())
    {
        std::getline(f, line);
        auto l0 = line.substr(0, 1);
        if (l0.compare("c") == 0) continue;
        auto parsed = split(line, default_delim);
        if (l0.compare("p") == 0) // format: p col <n_vertices> <n_edges>
        {
            n_vertices = std::atoll(parsed[2].c_str());
//            n_edges = std::atoll(parsed[3].c_str());
            adjacency_matrix.resize(n_vertices, vertex_array(n_vertices, 0));
        }
        if (l0.compare("e") == 0) // format: e <vertex1> <vertex2>
        {
            auto v1 = static_cast<vertex>(std::atoll(parsed[1].c_str())) - 1,
                 v2 = static_cast<vertex>(std::atoll(parsed[2].c_str())) - 1;
            adjacency_matrix[v1][v2]++;
            adjacency_matrix[v2][v1]++;
            vertex_degrees_map[v1]++;
            vertex_degrees_map[v2]++;
        }
    }

    std::vector<std::pair<vertex, std::size_t>> vertex_degrees;
    for (const auto& element : vertex_degrees_map)
    {
        vertex_degrees.emplace_back(element);
    }

    std::sort(vertex_degrees.begin(), vertex_degrees.end(), [] (auto& pair1, auto& pair2)
    {
        return pair1.second > pair2.second;
    });

    start_time = _chrono::now();
    for (const auto& element : vertex_degrees)
    {
        clique q;
        q.m_candidates = get_connected(element.first);
        q.m_vertices.push_back(element.first);
        max_clique(q);
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(_chrono::now() - start_time);
    std::cout << elapsed.count() << " " << optimal_clique.m_vertices.size() << " " << pretty_print(optimal_clique) << std::endl;

    return 0;
}
catch (const std::exception&)
{
    std::cout << time_limit << " " << optimal_clique.m_vertices.size() << " " << pretty_print(optimal_clique) << std::endl;
    return 1;
}
