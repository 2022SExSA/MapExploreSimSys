#include <iostream>
#include "pg/pgtest/pgtest.h"
#include "jwt-cpp/jwt.h"

PGTEST_CASE(test_jwtcpp) {
	std::string token =
		"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXUyJ9.eyJpc3MiOiJhdXRoMCJ9.AbIJTDMFc7yUa5MhvcP03nJPyCPzZtQcGEp-zWfOkEE";
	auto decoded = jwt::decode(token);

	for (auto& e : decoded.get_payload_claims())
		std::cout << e.first << " = " << e.second << std::endl;

    return true;
}
