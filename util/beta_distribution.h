#pragma once

#include <random>

/**
 * Beta distribution generation borrowed from
 * https://stackoverflow.com/questions/15165202/random-number-generator-with-beta-distribution
 */

template <typename RealType = float>
class beta_distribution
{
public:
	typedef RealType result_type;

	explicit beta_distribution(RealType a = 2.0, RealType b = 2.0)
		: a_gamma(a), b_gamma(b) { }

	template <typename URNG>
	result_type operator()(URNG& engine)
	{
		return generate(engine, a_gamma, b_gamma);
	}

private:
	typedef std::gamma_distribution<result_type> gamma_dist_type;

	gamma_dist_type a_gamma, b_gamma;

	template <typename URNG>
	result_type generate(URNG& engine,
		gamma_dist_type& x_gamma,
		gamma_dist_type& y_gamma)
	{
		result_type x = x_gamma(engine);
		return x / (x + y_gamma(engine));
	}
};

