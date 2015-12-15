/**
 * @file sparse_bias_layer.hpp
 * @author Tham Ngap Wei
 *
 * Definition of the SparseBiasLayer class.
 */
#ifndef __MLPACK_METHODS_ANN_LAYER_SPARSE_BIAS_LAYER_HPP
#define __MLPACK_METHODS_ANN_LAYER_SPARSE_BIAS_LAYER_HPP

#include <mlpack/core.hpp>
#include <mlpack/methods/ann/layer/layer_traits.hpp>
#include <mlpack/methods/ann/init_rules/zero_init.hpp>
#include <mlpack/methods/ann/optimizer/rmsprop.hpp>

namespace mlpack {
namespace ann /** Artificial Neural Network. */ {

/**
 * An implementation of a bias layer design for sparse autoencoder.
 * The BiasLayer class represents the bias part of sparse autoencoder.
 * 
 *
 * @tparam OptimizerType Type of the optimizer used to update the weights.
 * @tparam WeightInitRule Rule used to initialize the weight matrix.
 * @tparam InputDataType Type of the input data (arma::colvec, arma::mat,
 *         arma::sp_mat or arma::cube).
 * @tparam OutputDataType Type of the output data (arma::colvec, arma::mat,
 *         arma::sp_mat or arma::cube).
 */
template <
    template<typename, typename> class OptimizerType = mlpack::ann::RMSPROP,
    class WeightInitRule = ZeroInitialization,
    typename InputDataType = arma::mat,
    typename OutputDataType = arma::mat
>
class SparseBiasLayer
{
 public:
  /**
   * Create the BiasLayer object using the specified number of units and bias
   * parameter.
   *
   * @param outSize The number of output units.
   * @param sampleSize The size of the training data(how many data for training)
   * @param bias The bias value.
   * @param WeightInitRule The weight initialization rule used to initialize the
   *        weight matrix.
   */
  SparseBiasLayer(const size_t outSize,
                  const size_t sampleSize,
                  WeightInitRule weightInitRule = WeightInitRule()) :
      outSize(outSize),
      sampleSize(sampleSize),
      optimizer(new OptimizerType<SparseBiasLayer<OptimizerType,
                                                  WeightInitRule,
                                                  InputDataType,
                                                  OutputDataType>,
                                                  InputDataType>(*this)),
      ownsOptimizer(true)
  {
    weightInitRule.Initialize(weights, outSize, 1);
  }
  
  SparseBiasLayer(SparseBiasLayer &&layer) noexcept
  {
    *this = std::move(layer);
  }

  SparseBiasLayer& operator=(SparseBiasLayer &&layer) noexcept
  {
    optimizer = layer.optimizer;    
    ownsOptimizer = layer.ownsOptimizer;
    layer.optimizer = nullptr;
    layer.ownsOptimizer = false;

    outSize = layer.outSize;   
    sampleSize = layer.sampleSize;
    weights.swap(layer.weights);
    delta.swap(layer.delta);
    gradient.swap(layer.gradient);
    inputParameter.swap(layer.inputParameter);
    outputParameter.swap(layer.outputParameter);

    return *this;
  }

  /**
   * Delete the bias layer object and its optimizer.
   */
  ~SparseBiasLayer()
  {
    if (ownsOptimizer)
      delete optimizer;
  }

  /**
   * Ordinary feed forward pass of a neural network, evaluating the function
   * f(x) by propagating the activity forward through f.
   *
   * @param input Input data used for evaluating the specified function.
   * @param output Resulting output activation.
   */
  template<typename eT>
  void Forward(const arma::Mat<eT>& input, arma::Mat<eT>& output)
  {    
    output = input + arma::repmat(weights, 1, input.n_cols);
  }

  /**
   * Ordinary feed backward pass of a neural network, calculating the function
   * f(x) by propagating x backwards trough f. Using the results from the feed
   * forward pass.
   *
   * @param input The propagated input activation.
   * @param gy The backpropagated error.
   * @param g The calculated gradient.
   */
  template<typename DataType, typename ErrorType>
  void Backward(const DataType& /* unused */,
                const ErrorType& gy,
                ErrorType& g)
  {
    g = gy;
  }  

  /*
   * Calculate the gradient using the output delta and the bias.
   *
   * @param d The calculated error.
   * @param g The calculated gradient.
   */
  template<typename eT>
  void Gradient(const arma::Mat<eT>& d, InputDataType& g)
  {
    using inputDataType = std::decay<decltype(inputParameter[0])>::type;
    g = arma::sum(d, 1) / static_cast<inputDataType>(sampleSize);    
  }

  //! Get the optimizer.
  OptimizerType<SparseBiasLayer<OptimizerType,
                          WeightInitRule,
                          InputDataType,
                          OutputDataType>, InputDataType>& Optimizer() const
  {
    return *optimizer;
  }
  //! Modify the optimizer.
  OptimizerType<SparseBiasLayer<OptimizerType,
                          WeightInitRule,
                          InputDataType,
                          OutputDataType>, InputDataType>& Optimizer()
  {
    return *optimizer;
  }

  //! Get the weights.
  InputDataType& Weights() const { return weights; }
  //! Modify the weights.
  InputDataType& Weights() { return weights; }

  //! Get the input parameter.
  InputDataType& InputParameter() const {return inputParameter; }
  //! Modify the input parameter.
  InputDataType& InputParameter() { return inputParameter; }

  //! Get the output parameter.
  OutputDataType& OutputParameter() const {return outputParameter; }
  //! Modify the output parameter.
  OutputDataType& OutputParameter() { return outputParameter; }

  //! Get the delta.
  OutputDataType& Delta() const {return delta; }
  //! Modify the delta.
  OutputDataType& Delta() { return delta; }

  //! Get the gradient.
  InputDataType& Gradient() const {return gradient; }
  //! Modify the gradient.
  InputDataType& Gradient() { return gradient; }

 private:
  //! Locally-stored number of output units.
  size_t outSize;

  //! Sample size of the training data
  size_t sampleSize;

  //! Locally-stored weight object.
  InputDataType weights;

  //! Locally-stored delta object.
  OutputDataType delta;

  //! Locally-stored gradient object.
  InputDataType gradient;

  //! Locally-stored input parameter object.
  InputDataType inputParameter;

  //! Locally-stored output parameter object.
  OutputDataType outputParameter;

  //! Locally-stored pointer to the optimzer object.
  OptimizerType<SparseBiasLayer<OptimizerType,
                          WeightInitRule,
                          InputDataType,
                          OutputDataType>, InputDataType>* optimizer;

  //! Parameter that indicates if the class owns a optimizer object.
  bool ownsOptimizer;
}; // class SparseBiasLayer

//! Layer traits for the SparseBiasLayer.
template<
  template<typename, typename> class OptimizerType,
  typename WeightInitRule,
  typename InputDataType,
  typename OutputDataType
>
class LayerTraits<SparseBiasLayer<
    OptimizerType, WeightInitRule, InputDataType, OutputDataType> >
{
 public:
  static const bool IsBinary = false;
  static const bool IsOutputLayer = false;
  static const bool IsBiasLayer = true;
  static const bool IsLSTMLayer = false;
  static const bool IsConnection = true;
};

} // namespace ann
} // namespace mlpack

#endif
