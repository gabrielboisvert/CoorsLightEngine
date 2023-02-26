#include <QtWidgets/qframe.h>
#include <QtWidgets/QGridLayout>
#include <QtCore/QParallelAnimationGroup>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

namespace Editor::Widget
{
    class WidgetCollapse : public QWidget
    {
        public:
            QGridLayout mainLayout;
            QToolButton toggleButton;
            QParallelAnimationGroup toggleAnimation;
            QScrollArea contentArea;
            int animationDuration{ 300 };

            WidgetCollapse(const QIcon& pIcon, const QString& title = "", const int animationDuration = 100, QWidget* parent = 0);
            void setContentLayout(QLayout& contentLayout);
    };
}