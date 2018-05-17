
#ifndef POPULATION_H
#define POPULATION_H


#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include "Matrix.h"
#include "NeuralNet.h"
#include "Player.h"
#include "TicTacToe.h"
#include "Genetic.h"

using std::cout;
using std::cin;
using std::endl;
using std::ostream;
using std::istream;
using std::vector;
using std::string;

template <template <class, class> class T>
class Population {
public:
    Population<T>();
    
    void init(unsigned int seed = 1, istream& is = cin, ostream& os = cout);
    void train(bool verbose);
    void saveBest(string path);
    void loadBest(string path);
    
private:
    int m_populationSize;
    int m_iterations;
    int m_hiddenLayers;
    vector<unsigned int> m_layerSizes;
    
    vector< playerContainer<NeuralPlayer> > m_population;
    vector< playerContainer<NeuralPlayer> > m_hallOfFame;
    
    Genetic m_ga;
    
    void savePlayerToFile(NeuralPlayer best, string path);
    playerContainer<NeuralPlayer> loadPlayerFromFile(string path);
    
    void roundRobin();
    double playHallOfFame(playerContainer<NeuralPlayer>& best);
    
    void printSummary(int generation, double HOF_percent);
    void printPopulationFrom(unsigned int start, unsigned int end);
};



template <template <class, class> class T>
void Population<T>::savePlayerToFile(NeuralPlayer best, string path){
    string fileName;
    
    cout << "Player datafile name (saved to '" << path << "'): " << endl;
    cin >> fileName;
    
    //Load the trained player
    while(!best.neural.saveToFile(path + fileName)){
        cout << "  Invalid file name or path. Please try again." << endl;
        cout << "Player datafile name (saved to '" << path << "'): " << endl;
        cin >> fileName;
    }
}

template <template <class, class> class T>
playerContainer<NeuralPlayer> Population<T>::loadPlayerFromFile(string path){
    string fileName;
    NeuralPlayer tempLoadedPlayer;
    
    cout << "Player datafile name (located in '" << path << "'): " << endl;
    cin >> fileName;
    
    //Load the trained player
    while(!tempLoadedPlayer.neural.loadFromFile(path + fileName)){
        cout << "  Invalid file name or path. Please try again." << endl;
        cout << "Player datafile name (located in '" << path << "'): " << endl;
        cin >> fileName;
    }
    
    playerContainer<NeuralPlayer> loadedPlayer(tempLoadedPlayer);
    return loadedPlayer;
}


template <template <class, class> class T>
void Population<T>::loadBest(string path){
    //Load player
    playerContainer<NeuralPlayer> loadedPlayer = loadPlayerFromFile(path);
    loadedPlayer.player.neural.printWeights();
    
    //Set up a human-input player
    ManualPlayer tempHuman(cin, cout);
    playerContainer<ManualPlayer> human(tempHuman);
    
    //Play each other
    T<ManualPlayer, NeuralPlayer> testGame(human, loadedPlayer, true);
    testGame.playGame();
}

template <template <class, class> class T>
void Population<T>::saveBest(string path){
    NeuralPlayer best = m_population.back().player;
    best.neural.printWeights();
    savePlayerToFile(best, path);
}


template <template <class, class> class T>
void Population<T>::train(bool verbose){
    for(int generation = 0; generation < m_iterations; ++generation){
        //Play games with every permutaiton of players
        roundRobin();
        
        //Sorts the players by fitness (ascending)
        sort(m_population.begin(), m_population.end(),
            comparePlayerContainer<NeuralPlayer>);
        
        m_hallOfFame.push_back(m_population.back());
        
        //printPopulationFrom(0, 10, m_population);
        //printPopulationFrom(0, populationSize, m_population);
        
        //Print board
        if(verbose){
            NeuralPlayer best = m_population.back().player;
            best.neural.printWeights();
        }
        
        //Test the current best vs. the best from each previous generation
        double HOF_percent = playHallOfFame(m_population.back());
        
        //Print epoch summary
        printSummary(generation, HOF_percent);
        
        //Make new players based on how successful the current ones are
        m_ga.breed(m_population);
        
        //Each weight has a small chance to change by some random value
        m_ga.mutate(m_population);
        
        //Reset fitness values for next generation
        for(int i = 0; i < m_populationSize; ++i){
            m_population[i].player.resetFitness();
        }
    }
}


template <template <class, class> class T>
Population<T>::Population() : m_ga(0.03f, 0.1f){
    
}

template <template <class, class> class T>
void Population<T>::init(unsigned int seed, istream& is, ostream& os){
    srand(seed);
    
    //Get population size
    os << "Population size: ";
    is >> m_populationSize;
    if(m_populationSize < 2){
        m_populationSize = 2;
    }
    
    //Get number of iterations
    os << "Iterations: ";
    is >> m_iterations;
    if(m_iterations < 1){
        m_iterations = 1;
    }
    
    //Get number of layers
    os << "Number of hidden layers: ";
    is >> m_hiddenLayers;
    
    //Populate m_layerSizes
    m_layerSizes.push_back(NUM_INPUTS);
    for(int i = 1; i <= m_hiddenLayers; ++i){
        os << "Number in hidden layer " << i << ": ";
        unsigned int temp;
        is >> temp;
        if(temp < 1){
            temp = 1;
        }
        m_layerSizes.push_back(temp);
    }
    m_layerSizes.push_back(NUM_OUTPUTS);
    
    //Instantiate the Players
    for(int i = 0; i < m_populationSize; ++i){
        NeuralPlayer temp(m_layerSizes);
        playerContainer<NeuralPlayer> tempContainer(temp);
        m_population.push_back(tempContainer);
    }
    
    m_ga.setPopulationSize(m_populationSize);
}

template <template <class, class> class T>
void Population<T>::roundRobin(){
    for(int i = 0; i < m_populationSize - 1; ++i){
        for(int j = i + 1; j < m_populationSize; ++j){
            //Game 1
            //cout << "Game between [" << population[i].second << "] and [" << population[j].second << "]" << endl;
            T<NeuralPlayer, NeuralPlayer> game1(m_population[i], m_population[j], false);
            game1.playGame();
            
            //Game 2 (play 2 games so both players can start first)
            //cout << "Game between [" << population[j].second << "] and [" << population[i].second << "]" << endl;
            T<NeuralPlayer, NeuralPlayer> game2(m_population[j], m_population[i], false);
            game2.playGame();
        }   
    }
}

template <template <class, class> class T>
double Population<T>::playHallOfFame(playerContainer<NeuralPlayer>& best){
    int numOpponents = m_hallOfFame.size() - 1;
    double initialFitness = best.player.getFitness();
    
    //Reset the best players' fitness to 0
    best.player.resetFitness();
    
    for(int i = 0; i < numOpponents; ++i){
        //Game 1
        T<NeuralPlayer, NeuralPlayer> game1(m_hallOfFame[i], best, false);
        game1.playGame();
        
        //Game 2 (play 2 games so both players can start first)
        T<NeuralPlayer, NeuralPlayer> game2(best, m_hallOfFame[i], false);
        game2.playGame();
    }
    double fractionOfWins = best.player.getFitness() / (2 * numOpponents);
    
    //Set the best player's fitness to what it was before this function
    best.player.resetFitness();
    best.player.addToFitness(initialFitness);
    
    return 100 * fractionOfWins;
}

template <template <class, class> class T>
void Population<T>::printSummary(int generation, double HOF_percent){
    
    playerContainer<NeuralPlayer> maxPlayer = m_population.back();
    playerContainer<NeuralPlayer> minPlayer = m_population.front();
    playerContainer<NeuralPlayer> medianPlayer = m_population[m_populationSize / 2];
    
    int maxPossible = 2 * (m_populationSize - 1);
    
    
    printf("Gen: %3d", generation);
    printf(",   Min: %-6.1f [i=%-3d]", 
        minPlayer.player.getFitness(), minPlayer.index);
    
    printf(",   Median: %-6.1f [i=%-3d]", 
        medianPlayer.player.getFitness(), medianPlayer.index);
    
    printf(",   Max: %-6.1f [i=%-3d]", 
        maxPlayer.player.getFitness(), maxPlayer.index);
    
    printf(",   Max possible: %4d", maxPossible);
    printf(",   Win vs HOF: %.2lf%%", HOF_percent);
    cout << endl;
}

template <template <class, class> class T>
void  Population<T>::printPopulationFrom(unsigned int start, unsigned int end){
    if (start > end){
        cerr << "Error: printPopulationFrom start is greater than end" << endl;
        exit(1);
    }
    if (end > m_population.size()){
        cerr << "Error: printPopulationFrom end is larger than populationSize" << endl;
        exit(1);
    }
    for(unsigned int i = start; i < end; ++i){
        cout << "Population[" << m_population[i].index << "] fitness: ";
        printf("%5.1f\n", m_population[i].player.getFitness());
    }
}

#endif

