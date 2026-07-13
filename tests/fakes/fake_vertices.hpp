/**
 * @file fake_vertices.hpp
 * @author Chris DeFrancisci (chrisdefrancisici@gmail.com)
 * @brief Simplified vertex algorithms that don't really do anything
 */

#include <reusable_synth/software/vertex_interface.hpp>

/**
 * @brief A vertex that uses a function object to initialize all of its
 * elements.
 *
 * Does not consume any data.
 *
 * @tparam DataType The data type of computations.
 * @tparam Size The number of data points.
 */
template<typename DataType, size_t Size>
class InitialVertex : public VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;
    InitialVertex(int id,
                  std::function<void(DataType&)> init,
                  std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
    {
        for (int i = 0; i < Size; i++) {
            init(this->getOutput(i));
        }
    }

    [[nodiscard]] auto getInputs() const
      -> const std::pmr::vector<ConnectFunc>& override
    {
        return inputs; // Should be empty
    }

    /**
     * @brief No-op
     *
     */
    void execute() override {}

private:
    std::pmr::vector<ConnectFunc> inputs;
};

/**
 * @brief A vertex that doubles the input it consumes.
 *
 * @tparam DataType The data type of computations.
 * @tparam Size The number of data points.
 */
template<typename DataType, size_t Size>
class DoubleInput : public VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;

    DoubleInput(int id, std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
    {
        inputs.push_back(
          static_cast<ConnectFunc>(&DoubleInput<DataType, Size>::input));
    }

    [[nodiscard]] auto getInputs() const
      -> const std::pmr::vector<ConnectFunc>& override
    {
        return inputs;
    }

    void execute() override
    {
        ASSERT_EQ(inputBuff.size(), Size);
        for (size_t i = 0; i < Size; i++) {
            this->getOutput(i) = inputBuff[i] * DataType(2);
        }
    }

private:
    std::span<const DataType>
      inputBuff{}; // Fixed size span does not have default ctor
    void input(std::span<const DataType, Size> outputBuff)
    {
        // I believe we need to static cast here because this will be invoked
        // using a VertexInterface pointer
        static_cast<DoubleInput<DataType, Size>*>(this)->inputBuff = outputBuff;
    }
    std::pmr::vector<ConnectFunc> inputs;
};

/**
 * @brief A vertex that consumes two inputs and produces their product.
 *
 * @tparam DataType The data type of computations.
 * @tparam Size The number of data points.
 */
template<typename DataType, size_t Size>
class MultiplyInputs : public VertexInterface<DataType, Size>
{
public:
    using typename VertexInterface<DataType, Size>::ConnectFunc;

    static constexpr size_t connectA = 0;
    static constexpr size_t connectB = 1;

    MultiplyInputs(int id, std::pmr::memory_resource* mem)
      : VertexInterface<DataType, Size>(id, mem)
    {
        inputs.push_back(
          static_cast<ConnectFunc>(&MultiplyInputs<DataType, Size>::inputA));
        inputs.push_back(
          static_cast<ConnectFunc>(&MultiplyInputs<DataType, Size>::inputB));
    }

    [[nodiscard]] auto getInputs() const
      -> const std::pmr::vector<ConnectFunc>& override
    {
        return inputs;
    }

    void execute() override
    {
        ASSERT_EQ(inputABuff.size(), Size);
        ASSERT_EQ(inputBBuff.size(), Size);
        for (size_t i = 0; i < Size; i++) {
            this->getOutput(i) = inputABuff[i] * inputBBuff[i];
        }
    }

private:
    std::span<const DataType>
      inputABuff{}; // Fixed size span does not have default ctor
    std::span<const DataType>
      inputBBuff{}; // Fixed size span does not have default ctor

    void inputA(std::span<const DataType, Size> outputBuff)
    {
        static_cast<MultiplyInputs<DataType, Size>*>(this)->inputABuff =
          outputBuff;
    }
    void inputB(std::span<const DataType, Size> outputBuff)
    {
        static_cast<MultiplyInputs<DataType, Size>*>(this)->inputBBuff =
          outputBuff;
    }

    std::pmr::vector<ConnectFunc> inputs;
};
