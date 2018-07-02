
#include "../include/Genetic.h"


//mutationRate is in the range [0, 1], greedyPercent is in the range [0, 1]
Genetic::Genetic(const float mutationRate, const float greedyPercent)
    : m_mutationRate(clamp(mutationRate, 0.0f, 1.0f))
    , m_greedyPercent(clamp(greedyPercent, 0.0f, 1.0f)) {
    m_populationSize = 0;
}

void Genetic::setPopulationSize(int populationSize) {
    m_populationSize = populationSize;
}

//Make new players based on how successful the current ones are
void Genetic::breed(vector<playerContainer<NeuralPlayer> >& population) {
    vector<playerContainer<NeuralPlayer> > newPop;
    newPop.reserve(m_populationSize);

    //Keep the best greedyPercent of last generation
    int numToKeep = (int)(m_greedyPercent * (float)m_populationSize);
    if (numToKeep < 1) {
        numToKeep = 1;
    }

    //Copy the players which are being kept from greedyPercent
    for (int i = 0; i < numToKeep; ++i) {
        newPop.push_back(population[m_populationSize - 1 - i]);
    }

    //Iterates over the remaining child elements
    for (int i = numToKeep; i < m_populationSize; ++i) {
        playerContainer<NeuralPlayer> parent1 = pickParent(population);
        playerContainer<NeuralPlayer> parent2 = pickParent(population);
        vector<MatrixXd> newWeights = crossOver(parent1, parent2);

        playerContainer<NeuralPlayer> temp(population[m_populationSize - 1 - i]);
        temp.player.neural.setWeights(newWeights);
        newPop.push_back(temp);
    }
    population = newPop;
}

void Genetic::mutate(vector<playerContainer<NeuralPlayer> >& population) {
    //Intialize random object for gaussian distribution (mean=0, dev=0.1)
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> distribution(0, 0.1);

    int numToKeep = (int)(m_greedyPercent * (float)m_populationSize);

    for (int i = numToKeep; i < m_populationSize; ++i) {
        vector<MatrixXd> weights = population[i].player.neural.getWeights();
        size_t layers = weights.size();

        //For each layer
        for (size_t lay = 0; lay < layers; ++lay) {
            int rows = weights[lay].rows();
            int cols = weights[lay].cols();

            //Randomly mutate each element
            for (int col = 0; col < cols; ++col) {
                for (int row = 0; row < rows; ++row) {
                    //Mutate with a certain chance
                    if (((float)rand() / float(RAND_MAX)) < m_mutationRate) {
                        weights[lay](row, col) += distribution(gen);
                    }
                }
            }
        }
        population[i].player.neural.setWeights(weights);
    }
}

playerContainer<NeuralPlayer> Genetic::pickParent(
    const vector<playerContainer<NeuralPlayer> >& population) const {
//The sum of all player's fitness within the population
    double totalPopulationFitness = 0;
    for (int i = 0; i < m_populationSize; ++i) {
        double cur = population[i].player.getFitness();
        totalPopulationFitness += cur;
    }

    //A random number in the range [0, totalPopulationFitness - 1]
    int threshold = rand() % (int)totalPopulationFitness;

    double sum = 0;
    for (int i = m_populationSize - 1; i >= 0; --i) {
        double curFitness = population[i].player.getFitness();

        //Add the current player's fitness until it reaches the threshold
        sum += curFitness;
        if (sum >= threshold) {
            return population[i];
        }
    }
    //Default, return the highest fitness player
    return population.back();
}

vector<MatrixXd> Genetic::crossOver(const playerContainer<NeuralPlayer>& parent1,
                                    const playerContainer<NeuralPlayer>& parent2) {
    vector<MatrixXd> weights1;
    vector<MatrixXd> weights2;

    //Parent 1
    weights1 = parent1.player.neural.getWeights();
    weights2 = parent2.player.neural.getWeights();

    size_t length = weights1.size();

    //For each layer
    for (size_t i = 0; i < length; ++i) {
        int rows = weights1[i].rows();
        int cols = weights1[i].cols();

        //Cross breed matrix
        for (int col = 0; col < cols; ++col) {
            for (int row = 0; row < rows; ++row) {
                //50% chance of being from parent1 or parent2
                if (rand() % 2 == 0) {
                    weights1[i](row, col) = weights2[i](row, col);
                }
            }
        }

    }
    return weights1;
}
