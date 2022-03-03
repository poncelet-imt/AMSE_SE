from argparse import ArgumentParser
import matplotlib.pyplot as plt
import csv

def readCsv(file, delimiter, **kwargs):
    with open(file, 'r') as f:
        dataIter = csv.reader(f, delimiter=delimiter)
        
        header = next(dataIter)

        data = {h: [] for h in header}

        for row in dataIter:
            for index, value in enumerate(row):
                data[header[index]].append(float(value))

        return data
        
def main():
    kwargs = vars(getArgs())

    data = readCsv(**kwargs)
    print(kwargs['draw'])

    if kwargs['xy']:
        plt.plot(data["x"], data["y"], label="robot")
        plt.xlabel("x")
        plt.ylabel("y")
    else:
        for name in kwargs['draw'] if kwargs['draw'] else data:
            if name != "t":
                plt.plot(data["t"], data[name], label=name)
        
    plt.legend()
    plt.grid()
    plt.show()
    


def getArgs():
    parser = ArgumentParser()
    parser.add_argument('file', help="file csv containing the data to show", metavar="file",type=str)

    parser.add_argument('-d', '--delimiter', help="delimiter use on the csv file",
        type=str, choices=["\t", ",", ";"], default=",")
    parser.add_argument('-c', '--comma', help="use if the value use comma and not point",
        action='store_true')
    parser.add_argument('--draw', metavar='D', help="specify the serie you want to draw",
        nargs='*')
    parser.add_argument('--xy', help="use if you have a x and y values and want to plot them",
        action='store_true')

    args = parser.parse_args()
    return args



if __name__ == "__main__":
    main()