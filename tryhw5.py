from uwimg import *

def softmax_model(inputs, outputs):
    l = [make_layer(inputs, outputs, SOFTMAX)]
    return make_model(l)
# LINEAR, LOGISTIC, RELU, LRELU, SOFTMAX
def neural_net(inputs, outputs):
    print inputs
    l = [   make_layer(inputs, 32, RELU),
            make_layer(32, outputs, SOFTMAX)]
    return make_model(l)

print("loading data...")
train = load_classification_data("mnist.train", "mnist.labels", 1)
test  = load_classification_data("mnist.test", "mnist.labels", 1)
print("done")
print

print("training model...")
batch = 128
iters = 1000
rate = .001
momentum = .9
decay = 10

m = neural_net(train.X.cols, train.y.cols)
train_model(m, train, batch, iters, rate, momentum, decay)
print("done")
print

print("evaluating model...")
print("training accuracy: %f", accuracy_model(m, train))
print("test accuracy:     %f", accuracy_model(m, test))


