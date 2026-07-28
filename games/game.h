#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QVariant>

// ─────────────────────────────────────────────
// کلاس abstract پایه برای همه‌ی بازی‌ها
// (Dots and Boxes, Nine Men's Morris, Fanorona)
//
// هر بازی باید از این کلاس ارث‌بری کنه و توابع
// pure virtual رو با منطق خودش پیاده کنه.
// این معماری از Polymorphism استفاده می‌کنه:
// GameLobbyWindow / GameBoardWindow فقط با یه
// Game* کار می‌کنن، بدون این‌که بدونن دقیقاً
// کدوم بازیه.
// ─────────────────────────────────────────────

class Game : public QObject
{
    Q_OBJECT

public:
    explicit Game(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Game() = default;

    // ── اطلاعات پایه ──────────────────────────
    // شماره‌ی بازیکنی که الان نوبتشه (0 یا 1)
    virtual int currentPlayer() const = 0;

    // ── منطق اصلی بازی ────────────────────────
    // یه حرکت رو امتحان می‌کنه. اگه معتبر بود انجامش
    // میده و true برمی‌گردونه، وگرنه false (بدون تغییر state)
    // moveData شکلش بسته به بازی فرق می‌کنه (مثلاً برای
    // Dots and Boxes: شماره‌ی خط، برای Nine Men's Morris:
    // مبدا/مقصد مهره)
    virtual bool makeMove(int player, const QVariant& moveData) = 0;

    // آیا بازی تموم شده؟
    virtual bool isGameOver() const = 0;

    // برنده کیه؟ 0 یا 1 برای بازیکن‌ها، -1 برای مساوی،
    // -2 یعنی بازی هنوز ادامه داره
    virtual int getWinner() const = 0;

    // وضعیت فعلی تخته، برای رسم توسط widget یا برای
    // فرستادن روی شبکه (باید بشه راحت به/از JSON تبدیلش کرد)
    virtual QVariant getBoardState() const = 0;

    // بازی رو به حالت اولیه برمی‌گردونه
    virtual void resetGame() = 0;

    // اسم بازی، برای نمایش توی UI
    virtual QString gameName() const = 0;

signals:
    // هر وقت state تخته عوض بشه (بعد از یه حرکت معتبر)
    void boardChanged();

    // وقتی بازی تموم بشه؛ winner طبق همون قرارداد getWinner()
    void gameEnded(int winner);
};

#endif // GAME_H
