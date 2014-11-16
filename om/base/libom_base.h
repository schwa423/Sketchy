
namespace om {

class Foo {
public:
    Foo();
    ~Foo();
    void Touch();
    int GetTouchCount() const { return touch_count_; }
private:
    int touch_count_ = 0;
};

}  // namespace om