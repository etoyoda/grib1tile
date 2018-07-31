# 概要
GRIB第１版電文が複数書かれたファイルを読み込んで地理院タイル互換のPNG画像を書き出す。
たぶんそんなプログラムは既存では存在しない。
現代ではGRIB第１版すべてに対応する意義はもうない。日本気象庁1.25度Thinned Gridデータにだけ対応することにしたい。
# 出力
## フォーマット
地理院タイル仕様はここ https://maps.gsi.go.jp/development/siyou.html
想定入力が1.25度解像度で、つまり赤道大円を288分割した格子点でしか値が与えられていない。
## ズームレベル
仮に「出力は入力未満の解像度でなければならない」と考えるならば出力はズームレベルゼロとなるが、それはあんまりだろう。
天気図等の通常用途ではズームレベル4以上が普通なので、せめてズームレベル2くらいに内挿しておかないと表示が理解困難になると予想する。
まあ、やってみないとわからない。

仮に、風を矢羽で表現するのであれば、格子点あたりxyそれぞれ30ピクセルくらい使うので、ズームレベル5くらいは必要になる。しかしそこまでするのであれば、ズームレベル3くらいでベクトルタイルなのではないか。

## 要素
* 高層では基本的に力学５要素（東西・南北風、高度、気温、相対湿度）が与えられる。
* 地上では力学５要素（東西・南北風、海面更正気圧、気温、相対湿度）、直近６時間降水量。
* 925, 850 hPa では相当温位がみたい。
* 力学もの（500 hPa 渦度、250 hPa 発散、850 hPa 収束、渦位とか）はよくわからない。いらないような気がする。
* 圏界面高度はほしい気もするが、精度に懸念あり。
* 風を東西南北成分表示にすべきか、風向風速にすべきかは悩みどころ。地表と250 hPa だけは風速がほしいので、そういうところだけ風速でやってみるか。
* FAX図の伝統で湿数表示にするか、相対湿度にするか。まあ大した問題ではない。
全部がメモリに入りきるかどうか。

## 入力

格子構造のメモとしてダンプを末尾に添付する。


```
> File A_HPIA89RJTD290000_C_RJTD_20180729033949_89.grib
> IDS001: 47524942       "GRIB"            Magick number "GRIB"
> IDS005: 00136A         4970              Total length of GRIB message
>                                          (icnluding Section 0)
> IDS008: 01             1                 GRIB edition number (currently 1)
> PDS001: 00001C         28                Length of section
> PDS004: 03             3                 GRIB tables Version No
> PDS005: 22             34                Originating centre
>   (= Tokyo (RSMC), Japan Meteorological Agency)
> PDS006: 04             4                 Generating process identification
>                                          number
> PDS007: 25             37                Grid definition
> PDS008: 80             128               Flag
>   (:bit 1 = 1: Section 2 included)
>   (:bit 2 = 0: Section 3 omitted)
> PDS009: 02             2                 Indicator of parameter
>   (= Pressure reduced to MSL [Pa])
> PDS010: 66             102               Indicator of type of level
>   (= Mean sea level)
> PDS011: 0000           0                 Height, pressure, etc. of levels
> PDS013: 12071D0000     -                 Reference time of data - date and time
>                                          of start of averaging or accumulation
>                                          period
> PDS018: 01             1                 Indicator of unit of time range
>   (= Hour)
> PDS019: 00             0                 P1: period of time
> PDS020: 00             0                 P2: period of time or time interval
> PDS021: 00             0                 Time range indicator
>   (= Forecast product valid for reference time + P1 (P1 > 0), or Uninitialized
>                                          analysis product for reference time
>                                          (P1=0), or Image product for reference
>                                          time (P1=0))
> PDS022: 0000           0                 Number included in calculation when
>                                          octet 21 refers to a statistical
>                                          process, such as average or
>                                          accumulation; otherwise set to zero
> PDS024: 00             0                 Number missing from calculation in
>                                          case of statistical process
> PDS025: 15             20180729T000000Z  Century of reference time of data
> PDS026: 00             0                 Sub-centre identification
>   (= No sub-centre)
> PDS027: 8001           32769             Units decimal scale factor
> GDS001: 0000B2         178               Length of section (octets)
> GDS004: 00             0                 Number of vertical coordinate
>                                          parameters
> GDS005: 21             33                Location of the list of vertical
>                                          coorditate parameters, if present, or
>                                          location of the list of numbers of
>                                          points in each row
> GDS006: 00             0                 Data representation type
> GDS007: FFFF           missing           Number of points along a parallel
> GDS009: 0049           73                Number of points along a meridian
> GDS011: 000000         0                 Latitude of first grid point
> GDS014: 050910         330000            Longitude of first grid point
> GDS017: 80             128               Resolution and component flags
>   (:bit 1 = 1: Direction increments given)
>   (:bit 2 = 0: Earth assumed sperical with radius 6367.47 km)
>   (:bit 5 = 0: Resolved u- and v-components of vector quantities relative to
>                                          easterly and northerly directions)
> GDS018: 015F90         90000             Latitude of last grid point
> GDS021: 00EA60         60000             Longitude of last grid point
> GDS024: FFFF           missing           i Direction increment
> GDS026: 04E2           1250              j Direction increment
> GDS028: 40             64                Scanning mode
>   (:bit 1 = 0: Points scan in +i direction)
>   (:bit 2 = 1: Points scan in +j direction)
>   (:bit 3 = 0: Adjacent points in i direction are consecutive)
> GDS033: 0049           73                Numbers of points in each row
> GDS035: 0049           73                Numbers of points in each row
> GDS037: 0049           73                Numbers of points in each row
> GDS039: 0049           73                Numbers of points in each row
> GDS041: 0049           73                Numbers of points in each row
> GDS043: 0049           73                Numbers of points in each row
> GDS045: 0049           73                Numbers of points in each row
> GDS047: 0049           73                Numbers of points in each row
> GDS049: 0048           72                Numbers of points in each row
> GDS051: 0048           72                Numbers of points in each row
> GDS053: 0048           72                Numbers of points in each row
> GDS055: 0047           71                Numbers of points in each row
> GDS057: 0047           71                Numbers of points in each row
> GDS059: 0047           71                Numbers of points in each row
> GDS061: 0046           70                Numbers of points in each row
> GDS063: 0046           70                Numbers of points in each row
> GDS065: 0045           69                Numbers of points in each row
> GDS067: 0045           69                Numbers of points in each row
> GDS069: 0044           68                Numbers of points in each row
> GDS071: 0043           67                Numbers of points in each row
> GDS073: 0043           67                Numbers of points in each row
> GDS075: 0042           66                Numbers of points in each row
> GDS077: 0041           65                Numbers of points in each row
> GDS079: 0041           65                Numbers of points in each row
> GDS081: 0040           64                Numbers of points in each row
> GDS083: 003F           63                Numbers of points in each row
> GDS085: 003E           62                Numbers of points in each row
> GDS087: 003D           61                Numbers of points in each row
> GDS089: 003C           60                Numbers of points in each row
> GDS091: 003C           60                Numbers of points in each row
> GDS093: 003B           59                Numbers of points in each row
> GDS095: 003A           58                Numbers of points in each row
> GDS097: 0039           57                Numbers of points in each row
> GDS099: 0038           56                Numbers of points in each row
> GDS101: 0037           55                Numbers of points in each row
> GDS103: 0036           54                Numbers of points in each row
> GDS105: 0034           52                Numbers of points in each row
> GDS107: 0033           51                Numbers of points in each row
> GDS109: 0032           50                Numbers of points in each row
> GDS111: 0031           49                Numbers of points in each row
> GDS113: 0030           48                Numbers of points in each row
> GDS115: 002F           47                Numbers of points in each row
> GDS117: 002D           45                Numbers of points in each row
> GDS119: 002C           44                Numbers of points in each row
> GDS121: 002B           43                Numbers of points in each row
> GDS123: 002A           42                Numbers of points in each row
> GDS125: 0028           40                Numbers of points in each row
> GDS127: 0027           39                Numbers of points in each row
> GDS129: 0026           38                Numbers of points in each row
> GDS131: 0024           36                Numbers of points in each row
> GDS133: 0023           35                Numbers of points in each row
> GDS135: 0021           33                Numbers of points in each row
> GDS137: 0020           32                Numbers of points in each row
> GDS139: 001E           30                Numbers of points in each row
> GDS141: 001D           29                Numbers of points in each row
> GDS143: 001C           28                Numbers of points in each row
> GDS145: 001A           26                Numbers of points in each row
> GDS147: 0019           25                Numbers of points in each row
> GDS149: 0017           23                Numbers of points in each row
> GDS151: 0016           22                Numbers of points in each row
> GDS153: 0014           20                Numbers of points in each row
> GDS155: 0013           19                Numbers of points in each row
> GDS157: 0011           17                Numbers of points in each row
> GDS159: 0010           16                Numbers of points in each row
> GDS161: 000E           14                Numbers of points in each row
> GDS163: 000C           12                Numbers of points in each row
> GDS165: 000B           11                Numbers of points in each row
> GDS167: 0009           9                 Numbers of points in each row
> GDS169: 0008           8                 Numbers of points in each row
> GDS171: 0006           6                 Numbers of points in each row
> GDS173: 0005           5                 Numbers of points in each row
> GDS175: 0003           3                 Numbers of points in each row
> GDS177: 0002           2                 Numbers of points in each row
> BDS001: 001290         4752              Length of section
> BDS004: 0B             11                Flag
>   (:bit 1 = 0: Grid-point data)
>   (:bit 2 = 0: Simple pakcing)
>   (:bit 3 = 0: Floating point values (in the original data) are represented)
>   (:bit 4 = 0: No additional flags at octet 14)
> BDS005: 0000           0                 Scale factor
> BDS007: 44255E00                 9566    Reference value
> BDS011: 0B             11                Number of bits containing each packed
>                                          data
```
