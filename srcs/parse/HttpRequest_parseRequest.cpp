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
void HttpRequest::ParseRequest(const char* payload) {
  try {
    buffer_ += payload;

    for (;;) {
      switch (progress_) {
        case kHeader:
          if (!AdvanceHeaderParsing()) return; // still need more data
          continue; // after header is parsed, re-evaluate progress

        case kBody:
          if (!AdvanceBodyParsing()) return;
          return; // if body is parsed, progress is set to DONE
        case kDone:
          throw lib::exception::ResponseStatusException(lib::http::kBadRequest); // extra data after done
      }
    }
  }
  catch (lib::exception::ResponseStatusException& e) { throw; }
  catch (std::exception&) {
    throw lib::exception::ResponseStatusException(lib::http::kInternalServerError);
  }
}
