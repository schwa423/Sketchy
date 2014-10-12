
namespace om {

class Foo {
public:
    Foo();
    ~Foo();
    void Touch();
private:
    int touch_count_ = 0;
};

}  // namespace om