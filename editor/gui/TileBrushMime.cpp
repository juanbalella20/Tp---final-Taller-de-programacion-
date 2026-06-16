#include "TileBrushMime.h"

#include <QDataStream>
#include <QIODevice>
#include <QMimeData>

#include <utility>

QByteArray encode_tile_brush(const TileBrush &brush) {
  QByteArray encoded;
  // Escribe el contenido del brush al formato binario de QDataStream
  QDataStream stream(&encoded, QIODevice::WriteOnly);
  stream << static_cast<qint32>(brush.width)
         << static_cast<qint32>(brush.height);
  for (int gid : brush.gids)
    stream << static_cast<qint32>(gid);
  return encoded;
}

bool decode_tile_brush(const QMimeData *mime_data, TileBrush *brush) {
  if (!mime_data || !brush || !mime_data->hasFormat(TILE_BRUSH_MIME_TYPE))
    return false;

  QByteArray encoded = mime_data->data(TILE_BRUSH_MIME_TYPE);
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  qint32 width = 0;
  qint32 height = 0;
  // Se lee el width y el height
  stream >> width >> height;
  if (stream.status() != QDataStream::Ok || width <= 0 || height <= 0)
    return false;

  // Se calcula cuantos gid debería haber
  const qsizetype count =
      static_cast<qsizetype>(width) * static_cast<qsizetype>(height);
  if (count > WxH_LIMIT)
    return false;

  TileBrush decoded;
  decoded.width = width;
  decoded.height = height;
  decoded.gids.reserve(static_cast<std::size_t>(count));
  for (qsizetype i = 0; i < count; ++i) {
    qint32 gid = 0;
    stream >> gid;
    if (stream.status() != QDataStream::Ok || gid < 0)
      return false;
    decoded.gids.push_back(gid);
  }

  if (!stream.atEnd())
    return false;

  *brush = std::move(decoded);
  return true;
}
