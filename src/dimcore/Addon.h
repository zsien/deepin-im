#ifndef ADDON_H
#define ADDON_H

#include <QObject>

class Dim;

class Addon : public QObject {
    Q_OBJECT

public:
    explicit Addon(Dim *dim);
    virtual ~Addon();

    Dim *dim();

private:
    Dim *dim_;
};

#define DIM_ADDON_FACTORY(name)                                                                                        \
    extern "C" {                                                                                                       \
    Addon *create(Dim *dim) { return new name(dim); }                                                                  \
    }

typedef Addon *addonCreate(Dim *);

#endif // !ADDON_H