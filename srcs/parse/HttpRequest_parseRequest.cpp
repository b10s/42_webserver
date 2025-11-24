#include "HttpRequest.hpp"

/**
 * @brief HTTPリクエスト全体を段階的に解析するメイン関数。
 * 
 * @param payload 新しく受信したデータ（NULL終端文字列）
 * 
 * @throw http::responseStatusException
 *        - BAD_REQUEST: リクエスト完了後にさらに入力が来た場合など
 *        - INTERNAL_SERVER_ERROR: 解析処理中の想定外エラー
 * 
 * @note
 * - 不足データの場合は途中で return し、次回以降に続きを解析する。
 * - 完了時（DONE）はリクエスト全体のパースが成功した状態。
 */
void HttpRequest::parseRequest(const char* payload) {
  try {
    buffer_ += payload;

    for (;;) {
      switch (progress) {
        case HEADER:
          if (!advanceHeaderParsing()) return; // still need more data
          continue; // after header is parsed, re-evaluate progress

        case BODY:
          if (!advanceBodyParsing()) return;
          return; // if body is parsed, progress is set to DONE
        case DONE:
          throw http::responseStatusException(BAD_REQUEST);
      }
    }
  }
  catch (http::responseStatusException& e) { throw; }
  catch (std::exception&) {
    throw http::responseStatusException(INTERNAL_SERVER_ERROR);
  }
}
