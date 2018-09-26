#!/usr/bin/env python3

"""
Library for local search and initial solution
"""

# 2-opt
# relocation
# input: vector of paths
# output: new vector of paths
# can do optional: (found, new vector of paths)

import unittest

from lib.graph_utils import Solution

def construct_initial_solution(graph, ignore_constraints=False):
    """Construct Initial Solution given a GraphUtils object"""
    routes = []
    depot = graph.depot
    # TODO: optimize whole operation
    non_visited_customers = {c for c in graph.customers if not c.is_depot}
    for _ in range(graph.vehicle_number):
        if not non_visited_customers:
            break
        if len(routes) == graph.vehicle_number:
            break
        vehicle_route = [depot]
        route_capacity = float(graph.capacity)
        last_customer = vehicle_route[-1]
        possible_paths = sorted(
            [(i, cost) for i, cost in enumerate(graph.costs[last_customer]) if i in non_visited_customers],
            key=lambda x: x[1])
        while non_visited_customers and (route_capacity > possible_paths[0][1]):
            visited = set()
            for i, cost in possible_paths:
                if cost > route_capacity:
                    break
                next_customer = graph.costs[i]
                visited.add(next_customer)
                vehicle_route.append(next_customer)
                route_capacity -= cost
            non_visited_customers -= visited
        vehicle_route.append(depot)
        routes.append(vehicle_route)

    if not ignore_constraints:
        if len(routes) > graph.vehicle_number:
            raise ValueError('initial solution error: vehicle number exceeded')
        if max([len(path) for path in routes]) > graph.capacity:
            raise ValueError('initial solution error: capacity exceeded')
    return Solution(routes=routes)


# Unit Tests
class SearchUtilsTests(unittest.TestCase):
    """Unit Tests for search_utils methods"""

    BASIC_VRP = """
C108_shortened_x10

VEHICLE
NUMBER     CAPACITY
3         20

CUSTOMER
CUST NO.   XCOORD.   YCOORD.   DEMAND    READY TIME   DUE DATE   SERVICE TIME

    0      40         50          0          0       1236          0
    1      45         68         10        830       1049         90
    2      45         70         30        756        939         90
    3      42         66         10         16        336         90
    4      42         68         10        643        866         90
    5      42         65         10         15        226         90
    6      40         69         20        499        824         90
    7      40         66         20         87        308         90
    8      38         68         20        150        429         90
    9      38         70         10        429        710         90
"""

    def setUp(self):
        from graph_utils import GraphUtils
        from io import StringIO
        self.graph = GraphUtils(StringIO(SearchUtilsTests.BASIC_VRP))
        super(SearchUtilsTests, self).setUp()

    def test_construct_initial_solution_works(self):
        try:
            S = construct_initial_solution(self.graph, ignore_constraints=False)
            print(S)
        except Exception as e:
            self.fail(str(e))


if __name__ == '__main__':
    unittest.main()