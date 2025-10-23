#include "drawingstate.h"
#include "gui/views/imageviewer.h"

DrawingState::DrawingState(ImageViewer* viewer, QObject *parent)
    : QObject(parent), m_viewer(viewer)
{
}
