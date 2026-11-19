#ifndef VPD_H
#define VPD_H

// Tính SVP / VPD từ nhiệt độ và độ ẩm SHT30 (công thức Tetens / FAO-56).
// Các hàm ở đây chỉ tính từ tham số truyền vào: không đọc cảm biến, không gọi thư
// viện Arduino, không đụng biến toàn cục -> chạy test được ngay trên máy tính.
// CHỈ để theo dõi (gửi MQTT), không điều khiển relay nào. Gửi kèm topic hum-temp: {"vpd_air": 1.01, "vpd_leaf": 0.63} (kPa).
// leaf_offset đọc từ key "vpd_leaf_offset" trong config.json, ghi qua topic config:
//   {"key": "vpd_leaf_offset", "value": 2.0}
// Test: pio test -e native (test/test_vpd). Tài liệu đầy đủ: SmartGrowtentV2/docs/VPD.md

// ============================================================================
// SVP và VPD là gì
// ============================================================================
//
// SVP — Saturation Vapour Pressure (áp suất hơi bão hoà), kPa
//   Lượng hơi nước TỐI ĐA mà không khí chứa được ở một nhiệt độ, quy ra áp suất.
//   Chỉ phụ thuộc nhiệt độ, và tăng nhanh theo nhiệt độ (khoảng 1.8 lần mỗi 10 °C):
//     20 °C -> 2.34 kPa,  26 °C -> 3.36 kPa,  30 °C -> 4.24 kPa
//   Không khí nóng "chứa" được nhiều hơi nước hơn không khí lạnh.
//
// AVP — Actual Vapour Pressure (áp suất hơi thực tế), kPa
//   Lượng hơi nước ĐANG CÓ trong không khí:  AVP = SVP * RH / 100
//   Độ ẩm tương đối RH thực chất chỉ là tỉ lệ AVP / SVP.
//
// VPD — Vapour Pressure Deficit (độ thiếu hụt áp suất hơi), kPa
//   VPD = SVP - AVP: không khí còn "thiếu" bao nhiêu hơi nước nữa thì bão hoà.
//   Đây là lực kéo hơi nước từ trong lá ra ngoài (thoát hơi nước):
//     - VPD cao  -> không khí khô, lá thoát hơi mạnh. Quá cao thì cây đóng khí
//                   khổng để giữ nước -> ngừng hút CO2, chậm lớn, héo mép lá.
//     - VPD thấp -> không khí gần bão hoà, lá khó thoát hơi -> cây hút ít nước
//                   và dinh dưỡng từ rễ, dễ nấm mốc.
//
// Vì sao theo dõi VPD thay vì chỉ nhìn RH
//   Cùng RH 70%:  ở 20 °C thì VPD = 0.70 kPa,  ở 30 °C thì VPD = 1.27 kPa.
//   RH giống nhau mà với cây thì độ "khô" khác hẳn. VPD gộp nhiệt độ và độ ẩm
//   thành một con số, phản ánh đúng thứ cây cảm nhận.
//
// vpd_air và vpd_leaf
//   vpd_air  = SVP(T) - AVP          T là nhiệt độ không khí (SHT30 đo).
//   vpd_leaf = SVP(T_lá) - AVP       T_lá = T - leaf_offset.
//     Lá thường mát hơn không khí 1-3 °C (thoát hơi nước tự làm mát; đèn LED
//     ít bức xạ nhiệt nên lá càng mát). Bên trong lá coi như hơi nước bão hoà ở
//     nhiệt độ của lá, nên vpd_leaf sát với cây hơn, nhưng leaf_offset là số ƯỚC
//     LƯỢNG (không có cảm biến nhiệt độ lá). Mặc định 2 °C.
//   vpd_leaf ÂM: không khí chứa nhiều hơi nước hơn mức lá giữ được ở nhiệt độ
//     của lá -> hơi nước có xu hướng ĐỌNG lên lá, nguy cơ nấm cao. Vì vậy giá trị
//     âm được giữ nguyên, không kẹp về 0. Ví dụ 26 °C, RH 90%, lá mát hơn 2 °C
//     -> vpd_leaf ≈ -0.04 kPa.
//
// Dải tham khảo (kPa) — theo bảng phổ biến của người trồng, tuỳ loại cây:
//   cây con / giâm cành 0.4-0.8 | sinh trưởng 0.8-1.2 | ra hoa 1.2-1.6
//   Dưới ~0.4: quá ẩm.  Trên ~1.6: quá khô.
//
// Công thức SVP: Tetens, dạng dùng trong FAO-56 (tiêu chuẩn của FAO cho tính
// thoát hơi nước):  SVP = 0.6108 * exp(17.27 * T / (T + 237.3)), T tính bằng °C.
// Đủ chính xác trong khoảng 0-50 °C, thừa cho tủ trồng cây.
// ============================================================================

#define VPD_DEFAULT_LEAF_OFFSET 2.0f  // °C, lá mát hơn không khí

// Áp suất hơi bão hoà, kPa. T tính bằng °C.
float vpdSvp(float tempC);

// VPD không khí, kPa. RH được kẹp vào [0, 100].
float vpdAir(float tempC, float rh);

// VPD lá, kPa. Có thể ÂM khi độ ẩm rất cao — cố ý không kẹp về 0.
float vpdLeaf(float tempC, float rh, float leafOffsetC);

// true khi T và RH là số dùng được (không NaN / inf).
bool vpdInputValid(float tempC, float rh);

#endif
