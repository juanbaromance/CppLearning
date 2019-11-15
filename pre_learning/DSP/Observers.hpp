#pragma once

struct ExponentialSmoother
{

    /* https://en.wikipedia.org/wiki/Exponential_smoothing */
    ExponentialSmoother(string name, int N = 1)
    {
        _name = name;
        yk_1 = 0;
        plateau(N);
    }

    inline void reset(float value = 0) { yk_1 = value; }
    inline int plateau(int N = -1)
    {
        if (N < 1)
            return (static_cast<int>(round(1 / alpha_decay)));
        return (static_cast<int>(round((1 / (alpha_decay = 1. / N)))));
    }

    inline float update(float xk, float aux = 100000)
    {
        yk_1 = (aux == 100000) ? yk_1 : aux;
        return (yk_1 += alpha_decay * (xk - yk_1));
    }

protected:
    string _name;
    float yk_1, alpha_decay;
};
