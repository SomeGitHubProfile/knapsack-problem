#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <random>
#include <ctime>
#include <chrono>
#include <omp.h>

#define POPULATION_SIZE 800 // размер популяции (число особей определяет разнообразие решений в каждом поколении)
#define GENERATIONS 100 // число поколений - определяет время жизни популяции, чем больше, тем больше шансов получить хорошее решение
#define MUTATION_RATE 0.1 // вероятность мутации - мутация позволяет выпрыгнуть из локального экстремума, но мутации не должны быть слишком частыми, чтобы решение сходилось. При частых мутациях решение будет слабо отличаться от случайного
#define BATTLE_SIZE 18 // число особей, участвующих в "битве" - имитация естественного отбора и борьбы за выживание
#define TOURNAMENT_SIZE 100 // число особей, участвующих в борьбе за выбор родителя

using namespace std;

struct Item {
    int price;
    int weight;
};

// особь
struct Individual {
    vector<bool> DNA; // ДНК - это какие предметы взяты в рюкзак
    int fitness; // приспособленность к выживанию - это суммарная стоимость, если вес не превышает максимального, или 0 в противном случае

    Individual() noexcept : fitness(0) {}

    Individual(int max_weight, const vector<Item>& items, const vector<bool>& individual_dna) noexcept :
        DNA(individual_dna),
        fitness(0) {
        int price = 0;
        int weight = 0;
        for (int i = 0; i < items.size(); ++i) {
            if (DNA[i]) {
                price += items[i].price;
                weight += items[i].weight;
            }
        }

        if (weight <= max_weight) {
            fitness = price;
        }
    }
};

int solve(int max_weight, const vector<Item>& items) noexcept {
    vector<Individual> population(POPULATION_SIZE);

#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        mt19937 random_generator(random_device{}() + thread_id);

#pragma omp for
        for (int i = 0; i < POPULATION_SIZE; ++i) {
            vector<bool> dna(items.size());
            for (int j = 0; j < items.size(); ++j) {
                dna[j] = random_generator() & 1;
            }
            population[i] = Individual(max_weight, items, dna);
        }
    }

    for (int generation = 0; generation < GENERATIONS; ++generation) {
        vector<Individual> new_population(POPULATION_SIZE);

#pragma omp parallel
        {
            int thread_id = omp_get_thread_num();
            mt19937 random_generator(random_device{}() + thread_id + generation * 13);

            auto random_index = [&random_generator]() {
                return random_generator() % POPULATION_SIZE;
            };

            auto random_double = [&random_generator]() {
                return uniform_real_distribution<double>(0, 1)(random_generator);
            };

            auto mutate_dna = [&random_double](vector<bool> dna) {
                for (int i = 0; i < dna.size(); ++i) {
                    if (random_double() < MUTATION_RATE) {
                        dna[i] = !dna[i];
                    }
                }
                return dna;
            };

            auto tournament_selection = [&random_index , &population]() {
                Individual best = population[random_index()];
                for (int i = 1; i < TOURNAMENT_SIZE; ++i) {
                    Individual challenger = population[random_index()];
                    if (best.fitness < challenger.fitness) {
                        best = challenger;
                    }
                }
                return best;
            };

#pragma omp for
            for (int i = 0; i < POPULATION_SIZE; i += 2) {
                Individual mother = tournament_selection();
                Individual father = tournament_selection();

                vector<bool> son_dna(items.size());
                vector<bool> daughter_dna(items.size());

                for (int j = 0; j < items.size(); ++j) {
                    if (random_generator() & 1) {
                        son_dna[j] = father.DNA[j];
                        daughter_dna[j] = mother.DNA[j];
                    } else {
                        son_dna[j] = mother.DNA[j];
                        daughter_dna[j] = father.DNA[j];
                    }
                }

                new_population[i] = Individual(max_weight, items, mutate_dna(son_dna));
                if (i + 1 < POPULATION_SIZE) {
                    new_population[i + 1] = Individual(max_weight, items, mutate_dna(daughter_dna));
                }
            }
        }

        population = move(new_population);
    }

    int best_price = 0;
    for (const auto& individual : population) {
        best_price = max(best_price, individual.fitness);
    }

    return best_price;
}

vector<Item> manual_input(int& n, int& max_weight) noexcept {
    cout << "Input number of items: ";
    cin >> n;
    cout << "Input max backpack weight: ";
    cin >> max_weight;
    cout << "Input n pairs: price - weight\n";
    vector<Item> items(n);
    for (int i = 0; i < n; ++i) {
        cin >> items[i].price >> items[i].weight;
    }
    return items;
}

vector<Item> file_input(int& n, int& max_weight, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: can't open the file '" << filename << "'.\n";
        return {};
    }

    file >> n >> max_weight;
    vector<Item> items(n);
    for (int i = 0; i < n; ++i) {
        file >> items[i].price >> items[i].weight;
    }
    return items;
}

vector<Item> input_data(int& n, int& max_weight) {
    string filename;
    cout << "Enter input filename or press Enter to manually input data: ";
    getline(cin, filename);
    if (filename.empty()) {
        return manual_input(n, max_weight);
    }
    return file_input(n, max_weight, filename);
}

int main() {
    int n, max_weight;
    vector<Item> items = input_data(n, max_weight);
    auto start = chrono::high_resolution_clock::now();
    int best_price = solve(max_weight, items);
    auto end = chrono::high_resolution_clock::now();
    auto time_spent = end - start;
    cout << "Maximum price that can be taken: " << best_price << '\n';
    cout << "Time spent:\n";
    cout << "t = " << time_spent.count() << " nanoseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::milliseconds>(time_spent).count() << " milliseconds\n";
    cout << "t ~ " << chrono::duration_cast<chrono::seconds>(time_spent).count() << " seconds\n";
#if defined(NDEBUG)
    std::cout << "Press Enter to exit...";
    std::cin.ignore();
    std::cin.get();
#endif
    return 0;
}