import webpack from 'webpack';
import path from 'path';
import HtmlWebpackPlugin from 'html-webpack-plugin';
import UglifyjsWebpackPlugin from 'uglifyjs-webpack-plugin';
import CopyWebpackPlugin from 'copy-webpack-plugin';

const ENV = process.env.NODE_ENV || 'development';
const DISCORD_INVITE = process.env.DISCORD_INVITE || 'https://discord.com';

module.exports = {
  context: path.resolve(__dirname, 'src'),
  entry: './index.jsx',
  mode: ENV,

  output: {
    path: path.resolve(__dirname, 'dist'),
    publicPath: '/',
    filename: 'bundle.js',
  },

  resolve: {
    extensions: ['.jsx', '.js', '.sass', '.scss', '.json'],
  },

  module: {
    rules: [
      {
        test: /\.jsx?$/,
        exclude: path.resolve(__dirname, 'src'),
        enforce: 'pre',
        use: 'source-map-loader',
      },
      {
        test: /.jsx?$/,
        exclude: /node_modules/,
        use: 'babel-loader',
      },
      {
        test: /\.s[ac]ss$/i,
        use: [
          'style-loader',
          'css-loader',
          'sass-loader',
        ],
      },
      {
        test: /\.json$/,
        use: 'json-loader',
      },
      {
        test: /\.(xml|html|txt|md)$/,
        use: 'raw-loader',
      },
      {
        test: /\.(svg|woff2?|ttf|eot|jpe?g|png|gif)(\?.*)?$/i,
        use: ENV === 'production' ? 'file-loader' : 'url-loader',
      },
    ],
  },

  plugins: [
    new webpack.NoEmitOnErrorsPlugin(),
    new HtmlWebpackPlugin({
      template: './index.html',
      minify: { collapseWhitespace: true },
    }),
    new webpack.DefinePlugin({
      'ENV': JSON.stringify(ENV),
      'DISCORD_INVITE': JSON.stringify(DISCORD_INVITE)
    }),
  ].concat(ENV === 'development' ? [
    new CopyWebpackPlugin({
      patterns: [
        { from: './test-servers.json', to: './servers.json' },
      ],
    }),
  ] : []),

  optimization: {
    minimizer: ENV === 'production' ? [new UglifyjsWebpackPlugin()] : [],
  },

  devtool: ENV === 'production' ? false : 'eval-cheap-source-map',

  devServer: {
    port: process.env.PORT || 8080,
    host: 'localhost',
    publicPath: '/',
    contentBase: './src',
    historyApiFallback: {
      rewrites: [
        { from: /.*/, to: '/index.html' },
      ]
    }
  },
};
